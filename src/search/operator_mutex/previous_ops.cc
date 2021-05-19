#include "previous_ops.h"
#include "../utils/logging.h"
#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "reachability_strategy.h"
#include "labeled_transition.h"
#include "bdd_utils.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

namespace op_mutex {

unordered_set<OpMutex> PreviousOps::get_label_group_mutexes(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes) {
    unordered_set<OpMutex> label_group_mutexes;
    label_group_mutexes.reserve(ler->get_size() * ler->get_size());

    for (int i = 0; i < ler->get_size(); i++) {
        for (int j = i + 1; j < ler->get_size(); j++) {
            const LabelGroup gi = ler->get_group(i);
            const LabelGroup gj = ler->get_group(j);

            for (int a : gi) {
                for (int b : gj) {
                    if (label_mutexes.find(OpMutex(a, b)) == label_mutexes.end())
                        goto not_label_group_mutex;
                }
            }

            label_group_mutexes.emplace(i, j);

            not_label_group_mutex:;
        }
    }

    return label_group_mutexes;
}

vector<OpMutex> PreviousOps::get_label_group_mutexes_vector(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes) {
    vector<OpMutex> label_group_mutexes;
    label_group_mutexes.reserve(ler->get_size() * 4);

    for (int i = 0; i < ler->get_size(); i++) {
        for (int j = i + 1; j < ler->get_size(); j++) {
            const LabelGroup gi = ler->get_group(i);
            const LabelGroup gj = ler->get_group(j);

            for (int a : gi) {
                for (int b : gj) {
                    if (!label_mutexes.count(OpMutex(a, b)))
                        goto not_label_group_mutex;
                }
            }

            label_group_mutexes.emplace_back(i, j);

            not_label_group_mutex:;
        }
    }

    return label_group_mutexes;
}

void NoPO::run(CondensedTransitionSystem&, std::shared_ptr<LabelEquivalenceRelation>, unordered_set<OpMutex>&){ }

DynamicBitset<> NeLUSPO::find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) {
    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    std::unordered_set<int> ready_states;

    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    vector<DynamicBitset<>> state_labels = vector<DynamicBitset<>>(cts.num_abstract_states, DynamicBitset<>(num_label_groups));
    std::vector<bool> has_visited(cts.num_abstract_states, false);

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);

        std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (LabeledTransition t  : outgoing_transitions) {
            if (t.target == state)
                continue;

            remaining_parents[t.target]--;
            if (remaining_parents[t.target] == 0)
                ready_states.insert(t.target);

            if (!has_visited[t.target]) {
                state_labels[t.target] |= state_labels[state];
                state_labels[t.target].set(t.label_group);
                has_visited[t.target] = true;
            } else {
                //utils::g_log << "Current label group" << t.label_group << endl;
                auto temp = DynamicBitset<>(num_label_groups);
                temp.set(t.label_group);
                temp |= state_labels[state];
                state_labels[t.target] &= temp;
            }
        }
    }

    DynamicBitset<> unreachable_states(cts.num_abstract_states);

    for (int state = 0; state < cts.num_abstract_states; state++) {
        DynamicBitset<> necessary_labels = state_labels[state];

        for (size_t l1 = 0; l1 < necessary_labels.size(); l1++) {
            for (size_t l2 = l1 + 1; l2 < necessary_labels.size(); l2++) {
                if (necessary_labels[l1] && necessary_labels[l2] && label_group_mutexes.count(OpMutex(l1, l2))) {
                    goto unreachable;
                }
            }
        }
        continue;

        unreachable:
        unreachable_states.set(state);
    }

    return unreachable_states;
}

void PreviousOps::count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state) {
    std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);

    for (LabeledTransition t : outgoing_transitions) {
        if (t.target == state)
            continue;

        if (parents[t.target] == 0) {
            count_parents(cts, parents, t.target);
        }

        parents[t.target]++;
    }
}

void UnreachableStatesPreviousOps::run(CondensedTransitionSystem &cts, shared_ptr<LabelEquivalenceRelation> ler,
                                       unordered_set<OpMutex> &label_mutexes){

    DynamicBitset<> unreachable_states = find_unreachable_states(cts, get_label_group_mutexes(ler, label_mutexes), ler->get_size());

    vector<LabeledTransition> new_transitions;
    //utils::g_log << "Goal states size: " <<cts.abstract_goal_states.size() << endl;

    for (int s = 0; s < cts.num_abstract_states; s++) {
        if (unreachable_states[s] == 0) {
            for (LabeledTransition t : cts.get_abstract_transitions_from_state(s)) {
                new_transitions.push_back(t);
            }
        }
    }

    cts.abstract_transitions = new_transitions;
    sort(cts.abstract_transitions.begin(), cts.abstract_transitions.end());
}

DynamicBitset<> NaSUSPO::find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_labels) {
    DynamicBitset<> path(num_labels);
    DynamicBitset<> fully_reachable(cts.num_abstract_states);

    unreachable_states_dfs(cts, cts.initial_abstract_state, path, fully_reachable, label_group_mutexes);

    return ~fully_reachable;
}

void NaSUSPO::unreachable_states_dfs(
        const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
        const unordered_set<OpMutex> &label_group_mutexes)
{
    if (cts.abstract_goal_states.count(state))
        fully_reachable.set(state);

    vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);

    for(LabeledTransition &t : outgoing_transitions) {
        if(t.target == state)
            continue;

        // Check that this path is valid (i.e. it contains no two operators that are op-mutex)
        DynamicBitset<> labels_in_path(path.size());
        labels_in_path |= path;
        labels_in_path.set(t.label_group);

        bool is_label_group_mutex = false;
        for(size_t path_label_group = 0; path_label_group < path.size(); path_label_group++){
            // Skip this label if it is not in path
            if (!path[path_label_group])
                continue;

            if(label_group_mutexes.count(OpMutex(t.label_group, path_label_group))){
                is_label_group_mutex = true;
            }
        }
        // Do not consider this path to target state if the it is unreachable
        if (is_label_group_mutex)
            continue;

        bool prev_bit = path[t.label_group];
        path.set(t.label_group);
        unreachable_states_dfs(cts, t.target, path, fully_reachable, label_group_mutexes);

        // Only reset this bit if it was not set before
        if(!prev_bit)
            path.reset(t.label_group);

        if (fully_reachable[t.target]) {
            fully_reachable.set(state);
        }
    }
}

void UnreachableTransitionsPreviousOps::run(CondensedTransitionSystem &cts, shared_ptr<LabelEquivalenceRelation> ler,
                                       unordered_set<OpMutex> &label_mutexes){
    unordered_set<OpMutex> lgm = get_label_group_mutexes(ler, label_mutexes);

    vector<LabeledTransition> usable_transitions = find_usable_transitions(cts, lgm, ler->get_size());

    if (cts.abstract_transitions.size() - usable_transitions.size() > 0) {
        utils::g_log << "Unusable transitions: " << (cts.abstract_transitions.size() - usable_transitions.size()) << endl;

        cts.abstract_transitions = usable_transitions;
        sort(cts.abstract_transitions.begin(), cts.abstract_transitions.end());
    }
}

vector<LabeledTransition> NaSUTPO::find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) {
    DynamicBitset<> path(num_label_groups);
    unordered_set<LabeledTransition> usable_transitions;

    usable_transitions_dfs(cts, cts.initial_abstract_state, path, usable_transitions, label_group_mutexes);

    vector<LabeledTransition> ret;
    ret.reserve(usable_transitions.size());
    for (const LabeledTransition &t : usable_transitions) {
        ret.push_back(t);
    }

    return ret;
}

void NaSUTPO::usable_transitions_dfs(
        const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, unordered_set<LabeledTransition> &usable_transitions,
        const unordered_set<OpMutex> &label_group_mutexes)
{
    vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);

    for(LabeledTransition &t : outgoing_transitions) {

        // Check that this path is valid (i.e. it contains no two operators that are op-mutex)
        DynamicBitset<> labels_in_path(path.size());
        labels_in_path |= path;

        for(int path_label_group = 0; size_t(path_label_group) < path.size(); path_label_group++){
            // Skip this label if it is not in path
            if(path[path_label_group] && label_group_mutexes.count(OpMutex(t.label_group, path_label_group))){
                goto continue_outer;
            }
        }

        usable_transitions.insert(t);

        // Do not consider this path to target state if the it is unreachable
        if (t.src != t.target) {
            bool prev_bit = path[t.label_group];
            path.set(t.label_group);
            usable_transitions_dfs(cts, t.target, path, usable_transitions, label_group_mutexes);

            // Only reset this bit if it was not set before
            if(!prev_bit)
                path.reset(t.label_group);
        }
        continue_outer:;
    }
}

vector<LabeledTransition>
NeLUTPO::find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes,
                                 int num_label_groups) {
    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    // Initialize the ready_states to be the initial state (we do not care about any other states that start with indegree
    // 0, if they should exist.
    std::unordered_set<int> ready_states;
    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    // For each state we have a set of operator landmarks for it
    vector<DynamicBitset<>> state_labels = vector<DynamicBitset<>>(cts.num_abstract_states, DynamicBitset<>(num_label_groups));

    std::vector<bool> has_visited(cts.num_abstract_states, false);

    vector<LabeledTransition> usable_transitions;

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);

        std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (LabeledTransition t  : outgoing_transitions) {
            remaining_parents[t.target]--;
            if (remaining_parents[t.target] == 0)
                ready_states.insert(t.target);

            // Only consider this transition if it is usable
            if (!is_usable(state_labels[state], t, label_group_mutexes))
                continue;

            usable_transitions.push_back(t);

            // Do not compute necessary labels for self-loops
            if (t.src == t.target)
                continue;

            if (!has_visited[t.target]) {
                state_labels[t.target] |= state_labels[state];
                state_labels[t.target].set(t.label_group);
                has_visited[t.target] = true;
            } else {
                auto temp = DynamicBitset<>(num_label_groups);
                temp.set(t.label_group);
                temp |= state_labels[state];
                state_labels[t.target] &= temp;
            }

        }
    }

    return usable_transitions;
}

bool NeLUTPO::is_usable(const DynamicBitset<> &label_landmarks, LabeledTransition transition,
                        const unordered_set<OpMutex> &label_group_mutexes) {
    for (std::size_t i = 0; i < label_landmarks.size(); i++) {
        // If label i is used and i is op-mutex with the transitions label
        if (label_landmarks.test(i) && label_group_mutexes.count(OpMutex(i, transition.label_group))) {
            return false;
        }
    }
    return true;
}

void BDDOLMPO::SetSupersetHeavyBranch(vector<BitBDD>& bit_bdds) {
    for (auto & bit_bdd : bit_bdds)
        bit_bdd = BitBDD(bit_bdd.bdd.SupersetHeavyBranch(bit_bdd.used_vars.count(), 0), bit_bdd.used_vars);
}

void SetOverApprox(vector<BitBDD>& bit_bdds) {
    for (auto &bit_bdd : bit_bdds) {
        bit_bdd = BitBDD(bit_bdd.bdd.OverApprox(bit_bdd.used_vars.count(), 0), bit_bdd.used_vars);
    }
}

vector<LabeledTransition> BDDOLMPO::find_usable_transitions(CondensedTransitionSystem &cts,
                                                            vector<OpMutex> &label_group_mutexes,
                                                            int num_label_groups) {
    // If there are no label group mutexes, we just skip computation
    if (label_group_mutexes.empty())
        return cts.abstract_transitions;

    Cudd bdd_manager;
    init_bdd_manager(bdd_manager);

    // ---- Create op-mutex BDD ----
    // Sort on the smallest label group (has to be the first), this means that all label group mutexes that includes
    // label group 1 are group at the start, then all the remaining with label group 2 and so on...
    sort(label_group_mutexes.begin(), label_group_mutexes.end());

    // Find out which label groups have any op-mutex
    DynamicBitset<> label_group_has_op_mutex = DynamicBitset<>(num_label_groups);
    for (OpMutex &lgm : label_group_mutexes) {
        label_group_has_op_mutex.set(lgm.label1);
        label_group_has_op_mutex.set(lgm.label2);
    }

    vector<BitBDD> lgm_bdds(label_group_mutexes.size(), BitBDD(num_label_groups));
    {
        size_t i = 0;
        for (const OpMutex &m : label_group_mutexes) {
            lgm_bdds[i] = BitBDD(!(bdd_manager.bddVar(m.label1) * bdd_manager.bddVar(m.label2)),
                                 DynamicBitset<>(num_label_groups, {size_t(m.label1), size_t(m.label2)}));
            i++;
        }
    }

    merge2(bdd_manager, lgm_bdds, mergeAndBDD, max_bdd_time, max_bdd_size);

    if (approximate) {
        if (approximation_technique == 1) {
            SetSupersetHeavyBranch(lgm_bdds);
        } else if (approximation_technique == 2) {
            SetOverApprox(lgm_bdds);
        }
    }

    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    // Initialize the ready_states to be the initial state (we do not care about any other states that start with indegree
    // 0, if they should exist).
    std::unordered_set<int> ready_states;
    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    StateBDDs state_bdds(cts.num_abstract_states, vector<BitBDD>(1, BitBDD(bdd_manager.bddZero(),
                                                                                        DynamicBitset<>(num_label_groups))));

    state_bdds[cts.initial_abstract_state].push_back(BitBDD(bdd_manager.bddOne(), DynamicBitset<>(num_label_groups)));
//    utils::d_log << "state bdds precomputed: " << state_bdds_precomputed << endl;

//    utils::d_log << "Initial state: " << cts.initial_abstract_state << endl;

    vector<LabeledTransition> usable_transitions;

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);
//        utils::d_log << "State " << state << endl;

        std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (LabeledTransition t  : outgoing_transitions) {
//            utils::d_log << to_string(t) << endl;

            if (t.src != t.target) { // only decrement if the transition is not a self-loop
                remaining_parents[t.target]--;
                if (remaining_parents[t.target] == 0)
                    ready_states.insert(t.target);
            }

            if (label_group_has_op_mutex[t.label_group]) {
                BitBDDSet target_bdd_transition(state_bdds[state].size(), BitBDD(num_label_groups));
                for (size_t i = 0; i < state_bdds[state].size(); i++) {
                    target_bdd_transition[i] = state_bdds[state][i] * BitBDD(bdd_manager.bddVar(t.label_group), DynamicBitset<>(num_label_groups, {size_t(t.label_group)}));
                }
//              utils::d_log << "target_bdd_transition size: " << target_bdd_transition.size() << ", state_bdds[state] size: " << state_bdds->at(state).size() << endl;
                merge2(bdd_manager, target_bdd_transition, mergeOrBDD, max_bdd_time, max_bdd_size);
//              utils::d_log << "target_bdd_transition size after merge: " << target_bdd_transition.size() << endl;

                for (const BitBDD &lgm_bdd : lgm_bdds) {
                    for (const BitBDD &tbddt : target_bdd_transition)
                        if ((lgm_bdd.bdd * tbddt.bdd).IsZero()) {
                            goto not_usable;
                        }
                }

                if (t.src != t.target) {
//                  utils::d_log << "state_bdds.insert" << endl;
                    state_bdds[t.target].insert(state_bdds[t.target].end(), target_bdd_transition.begin(), target_bdd_transition.end());
                    merge2(bdd_manager, state_bdds[t.target], mergeOrBDD, max_bdd_time, max_bdd_size);
                }
            } else {
                if (t.src != t.target) {
//                  utils::d_log << "state_bdds.insert" << endl;
                    state_bdds[t.target].insert(state_bdds[t.target].end(), state_bdds[t.src].begin(), state_bdds[t.src].end());
                    merge2(bdd_manager, state_bdds[t.target], mergeOrBDD, max_bdd_time, max_bdd_size);
                }
            }

//            utils::d_log << "state_bdds[t.target] size: " << state_bdds->at(t.target).size() << endl;
//            utils::d_log << "Found usable transition: " << to_string(t) << endl;
            usable_transitions.push_back(t);

            not_usable:;
        }
    }

    return usable_transitions;
}



void BDDOLMPO::run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler,
                   unordered_set<OpMutex> &label_mutexes) {

    vector<OpMutex> lgm = get_label_group_mutexes_vector(ler, label_mutexes);

    vector<LabeledTransition> usable_transitions = find_usable_transitions(cts, lgm, ler->get_size());

    if (cts.abstract_transitions.size() - usable_transitions.size() > 0) {
        utils::g_log << "Unusable transitions: " << (cts.abstract_transitions.size() - usable_transitions.size()) << endl;

        cts.abstract_transitions = usable_transitions;
        sort(cts.abstract_transitions.begin(), cts.abstract_transitions.end());
    }
}

shared_ptr<NeLUSPO> _parse_neluspo(OptionParser &parser) {
    parser.document_synopsis(
            "NeLUSPO",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NeLUSPO>(opts);
}
static Plugin<PreviousOps> _plugin_neluspo("neluspo", _parse_neluspo);

shared_ptr<NeLUTPO> _parse_nelutpo(OptionParser &parser) {
    parser.document_synopsis(
            "NeLUTPO",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NeLUTPO>(opts);
}
static Plugin<PreviousOps> _plugin_nelutpo("nelutpo", _parse_nelutpo);

shared_ptr<BDDOLMPO> _parse_bddolmpo(OptionParser &parser) {
    parser.document_synopsis(
            "BDDOLMPO",
            "Reachability strategy");

    parser.add_option<int> (
            "max_bdd_size",
            "maximum size of mutex BDDs",
            "100");
    parser.add_option<int> (
            "max_bdd_time",
            "maximum time (ms) to generate mutex BDDs",
            "1");
    parser.add_option<bool>(
            "approximate",
            "boolean, indicating whether BDDOLMPO should approximate op-mutex BDDs.",
            "true");
    parser.add_option<int>(
            "approximation_technique",
            "1=heavy branch, 2=overapproximation",
            "2"
            );

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<BDDOLMPO>(opts);
}
static Plugin<PreviousOps> _plugin_bddolmpo("bddolmpo", _parse_bddolmpo);

shared_ptr<NaSUSPO> _parse_nasuspo(OptionParser &parser) {
    parser.document_synopsis(
            "NaSUSPO",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NaSUSPO>(opts);
}
static Plugin<PreviousOps> _plugin_nasuspo("nasuspo", _parse_nasuspo);

shared_ptr<NaSUTPO> _parse_nasutpo(OptionParser &parser) {
    parser.document_synopsis(
            "NaSUTPO",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NaSUTPO>(opts);
}
static Plugin<PreviousOps> _plugin_nasutpo("nasutpo", _parse_nasutpo);

shared_ptr<NoPO> _parse_nopo(OptionParser &parser) {
    parser.document_synopsis(
            "NoPO",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NoPO>(opts);
}
static Plugin<PreviousOps> _plugin_nopo("nopo", _parse_nopo);

static PluginTypePlugin<PreviousOps> _type_plugin("previous_ops", "The strategy for which to use previously found op-mutexes to find more.");
}
