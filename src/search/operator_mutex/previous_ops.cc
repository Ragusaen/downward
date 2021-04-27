#include "previous_ops.h"
#include "../utils/logging.h"
#include "operator_mutex_searcher.h"

#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "reachability_strategy.h"
#include "labeled_transition.h"
#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

namespace op_mutex {

unordered_set<OpMutex> PreviousOps::get_label_group_mutexes(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes) {
    unordered_set<OpMutex> label_group_mutexes;

    for (int i = 0; i < ler->get_size(); i++) {
        for (int j = i + 1; j < ler->get_size(); j++) {
            LabelGroup gi = ler->get_group(i);
            LabelGroup gj = ler->get_group(j);

            for (int a : gi) {
                for (int b : gj) {
                    if (!label_mutexes.count(OpMutex(a, b)))
                        goto not_label_group_mutex;
                }
            }

            label_group_mutexes.emplace(i, j);
            label_group_mutexes.emplace(j, i);

            not_label_group_mutex:;
        }
    }

    return label_group_mutexes;
}

void NoPO::run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes){
    // Dummy code to remove warnings 		(._.)/\(._.)
    auto a = cts.abstract_transitions.size() + ler->get_size() + label_mutexes.size();
    ++a;
}

DynamicBitset<> NeLUSPO::find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) {
    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    std::unordered_set<int> ready_states;

    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    vector<DynamicBitset<>> state_labels = vector<DynamicBitset<>>(cts.num_abstract_states, DynamicBitset<>(num_label_groups));
    std::vector<bool> has_visited(cts.num_abstract_states, false);

    //utils::g_log << "Num abstract states: " << cts.num_abstract_states << endl;

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);

        //utils::g_log << "Current state " << state << endl;

        std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (LabeledTransition t  : outgoing_transitions) {
            if (t.target == state)
                continue;

            //utils::g_log << "Current target " << t.target << endl;

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
                if (!label_group_mutexes.count(OpMutex(l1, l2))) {
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

    utils::g_log << "Found " << (fully_reachable.size() - fully_reachable.count()) << " unreachable states" << endl;

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
        else
            usable_transitions.insert(t);

        if (t.src == t.target) {
            continue;
        }

        bool prev_bit = path[t.label_group];
        path.set(t.label_group);
        usable_transitions_dfs(cts, t.target, path, usable_transitions, label_group_mutexes);

        // Only reset this bit if it was not set before
        if(!prev_bit)
            path.reset(t.label_group);
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
            if (t.target == state) {
                usable_transitions.push_back(t);
                continue;
            }

            remaining_parents[t.target]--;
            if (remaining_parents[t.target] == 0)
                ready_states.insert(t.target);

            // Only consider this transition if it is usable
            if (!is_usable(state_labels[state], t, label_group_mutexes))
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
            usable_transitions.push_back(t);
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

vector<LabeledTransition> BDDOLMPO::find_usable_transitions(CondensedTransitionSystem &cts,
                                                            const unordered_set<OpMutex> &label_group_mutexes,
                                                            int num_label_groups) {
    Cudd bdd_manager(num_label_groups);

    if (label_group_mutexes.empty())
        return cts.abstract_transitions;

    // Create op-mutex BDD
    utils::g_log << "Label group mutexes size: " << label_group_mutexes.size() << endl;

    vector<BDD> lgm_bdds(label_group_mutexes.size());
    int i = 0;
    for (const OpMutex &m : label_group_mutexes) {
        lgm_bdds[i] = !(bdd_manager.bddVar(m.label1) * bdd_manager.bddVar(m.label2));
        i++;
    }

    while (lgm_bdds.size() > 1) {
        size_t a = 1, b = 0;
        for (; a < lgm_bdds.size(); a += 2, b++) {
            lgm_bdds[b] = lgm_bdds[a] * lgm_bdds[a - 1];
        }
        if (a == lgm_bdds.size()) {
            lgm_bdds[b++] = lgm_bdds[a - 1];
        }

        lgm_bdds.resize(b);
    }

    BDD op_mutex_bdd = lgm_bdds.front();

    utils::g_log << "op_mutex_bdd node count after: " << op_mutex_bdd.nodeCount() << endl;

    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    // Initialize the ready_states to be the initial state (we do not care about any other states that start with indegree
    // 0, if they should exist.
    std::unordered_set<int> ready_states;
    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    std::vector<BDD> state_bdd(cts.num_abstract_states, bdd_manager.bddZero());
    state_bdd[cts.initial_abstract_state] = bdd_manager.bddOne();

    //utils::g_log << "Initial state: " << cts.initial_abstract_state << endl;

    vector<LabeledTransition> usable_transitions;

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);
        //utils::g_log << "State " << state << endl;

        std::vector<LabeledTransition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (LabeledTransition t  : outgoing_transitions) {
            //utils::g_log << to_string(t) << endl;

             state_bdd[state].nodeCount();

            remaining_parents[t.target]--;
            if (remaining_parents[t.target] == 0)
                ready_states.insert(t.target);

            if (!(state_bdd[state] * bdd_manager.bddVar(t.label_group) * op_mutex_bdd).IsZero()) {
                //utils::g_log << "Node count - " << "t.target: " << state_bdd[t.target].nodeCount() << " + (state: " << state_bdd[state].nodeCount() << " * bddVar: " << bdd_manager.bddVar(t.label_group).nodeCount() << ")" << endl;
                if (t.src != t.target) {
                    state_bdd[t.target] = state_bdd[t.target] + (state_bdd[state] * bdd_manager.bddVar(t.label_group));
                }

                usable_transitions.push_back(t);
            }
        }
    }

    return usable_transitions;
}
}