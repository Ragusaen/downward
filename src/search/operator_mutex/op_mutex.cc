#include "op_mutex.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include "../merge_and_shrink/merge_and_shrink_algorithm.h"
#include "../merge_and_shrink/transition_system.h"
#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "condensed_transition_system.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using options::Bounds;
using options::OptionParser;
using options::Options;
using utils::ExitCode;
using namespace merge_and_shrink;

template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig)
{
    std::vector<T> ret;
    for(const auto &v: orig)
        ret.insert(ret.end(), v.begin(), v.end());
    return ret;
}

namespace op_mutex_pruning {
OpMutexPruningMethod::OpMutexPruningMethod(const Options &opts)
{}

bool transition_label_comparison(Transition t, int l) { return t.label_group < l; }
bool transition_comparison(Transition a, int b) { return a.src < b; }

FactoredTransitionSystem* OpMutexPruningMethod::run(FactoredTransitionSystem* fts) {
    utils::g_log << "Operator mutex running" << endl;

    TransitionSystem ts = fts->get_transition_system(fts->get_size() - 1);
    int initial_state= ts.get_init_state();

    shared_ptr<LabelEquivalenceRelation> ler = ts.get_label_equivalence_relation();
    vector<vector<Transition>> tbg = ts.get_transitions();
    vector<Transition> grouped_transitions = std::vector<Transition>();

    for (int group = 0; group < ler->get_size(); group++) {
        for (Transition t : tbg[group]) {
            grouped_transitions.emplace_back(Transition(t.src, t.target, group));
        }
    }


    std::vector<Transition> k_ts = std::vector<Transition>();
    /*
    k_ts.push_back(Transition(0, 1));
    k_ts.push_back(Transition(0, 4));
    k_ts.push_back(Transition(1, 2));
    k_ts.push_back(Transition(2, 3));
    k_ts.push_back(Transition(4, 5));
    k_ts.push_back(Transition(4, 9));
    k_ts.push_back(Transition(5, 6));
    k_ts.push_back(Transition(6, 2));
    k_ts.push_back(Transition(6, 7));
    k_ts.push_back(Transition(7, 8));
    k_ts.push_back(Transition(9, 10));
    k_ts.push_back(Transition(10, 11));
    */

    utils::g_log << "Generating condensed transition system" << endl;
    CondensedTransitionSystem cts = CondensedTransitionSystem(grouped_transitions, ts.get_num_states(), initial_state);

    // Print the abstract transitions, if there are few, list them.
    if (cts.abstract_transitions.size() > 50) {
        utils::g_log << "Found abstract transitions: " << cts.abstract_transitions.size() << endl;
    } else {
        utils::g_log << "Abstract transitions: " << endl;
        for (Transition at : cts.abstract_transitions) {
            utils::g_log << to_string(at) << endl;
        }
    }

    utils::g_log << "Inferring operator mutexes" << endl;
    shared_ptr<Labels> labels = fts->get_labels_fixed();
    auto label_mutexes = infer_label_mutex_in_condensed_ts(cts, ler);

    utils::g_log << "Operator mutexes: " << label_mutexes.size() << endl;
    if (label_mutexes.size() < 10) {
        for (auto mutex : label_mutexes) {
            utils::g_log << mutex.first << " is operator mutex with " << mutex.second << "\t opnames: " << labels->get_name(mutex.first) << ", " << labels->get_name(mutex.second) << std::endl;
        }
    }

    utils::g_log << "Operator mutex done" << endl << endl;
    return fts;
}

#define REACH_XY(x, y) (reach[x * cts.num_abstract_states + y])
#define TRANS_IDX(idx) (cts.concrete_transitions[idx])
#define C2A(state) (cts.concrete_to_abstract_state[state])

/*
 * This function computes label mutexes, by checking for a pair of label groups (g1, g2) that all the transitions belonging
 * to g1 are transition-mutex with all transitions belonging to g2. Two transitions s_1 -l1> s_2, s_1' -l2> s_2' are transition-
 * mutex iff there is no path from s_2' to s_1 and from s_2 to s_1'. This check is done using a reachability matrix.
 */
std::vector<std::pair<int, int>>
OpMutexPruningMethod::infer_label_mutex_in_condensed_ts(
        CondensedTransitionSystem &cts, shared_ptr<LabelEquivalenceRelation> ler) {
    // Compute reachability between states
    vector<int> reach = vector<int>(cts.num_abstract_states * cts.num_abstract_states);
    // Start from the initial state of planning problem
    reachability(cts, reach, cts.initial_abstract_state);

    // Sort transitions on label group
    std::sort(cts.concrete_transitions.begin(), cts.concrete_transitions.end(),
              [](Transition a, Transition b){ return a.label_group < b.label_group; });

    // Find label_group mutexes
    vector<pair<int,int>> label_group_mutexes = vector<pair<int,int>>();

    int current_outer_label = 0;
    size_t to_end = 0;

    /*
     * The general idea of this loop is to iterate through all the transitions, but consider them within their label group.
     * The current_outer_label is the label that the outer loop is currently at, there are variable to_start and to_end
     * (to_end is exclusive) that correspond to the outer labels first and last transition in the vector. Then, for each
     * transition, starting from to_end as we only consider one of the two symmetric label pairs, we check if this
     * transition is transition mutex with all of the [to_start, to_end) transitions. When ti points to a new label, we
     * see if there were any non-transition-mutexes for the inner label, and if not we add it to the label mutex vector.
     * We skip all labels with 0 transitions, as these are trivially op-mutex with all other labels, and there is no need
     * to store all of them.
     */
    while (current_outer_label < ler->get_size() - 1) { // Do not consider last label
        int to_start = to_end; // End is exclusive

        // Search for the next label, could be binary search, but not really worth it
        for (; TRANS_IDX(to_end).label_group <= current_outer_label && to_end < cts.concrete_transitions.size(); to_end++);

        // Start looking from the next label group up, we only need to check half of the combinations because label
        // mutexes are symmetric
        int current_inner_label = current_outer_label + 1;

        size_t ti = to_end;

        bool is_label_mutex = true;
        for (; ti < cts.concrete_transitions.size(); ti++) {
            if (TRANS_IDX(ti).label_group > current_inner_label) {
                if (is_label_mutex) {
                    // Add only one of the two symmetric mutexes (we will add the other later)
                    label_group_mutexes.emplace_back(pair<int,int>(current_outer_label, current_inner_label));
                }

                // Start with next label
                current_inner_label = TRANS_IDX(ti).label_group;
                is_label_mutex = true;
            }

            for (size_t to = to_start; to < to_end; to++) {
                // If these two transitions are not mutex, the labels also aren't
                if (REACH_XY(C2A(TRANS_IDX(to).target), C2A(TRANS_IDX(ti).src))
                    || REACH_XY(C2A(TRANS_IDX(ti).target), C2A(TRANS_IDX(to).src)))
                {
                    is_label_mutex = false;
                    break;
                }
            }
        }
        if (to_end >= cts.concrete_transitions.size())
            break;
        current_outer_label = TRANS_IDX(to_end).label_group;
    }

    // Expand label groups into concrete labels
    vector<pair<int,int>> label_mutexes = vector<pair<int,int>>();
    for (auto gm : label_group_mutexes) {
        LabelGroup groupA = ler->get_group(gm.first);
        LabelGroup groupB = ler->get_group(gm.second);

        for (int la : groupA) {
            for (int lb : groupB) {
                label_mutexes.emplace_back(pair<int,int>(la, lb));
                label_mutexes.emplace_back(pair<int,int>(lb, la));
            }
        }
    }

    return label_mutexes;
}

void OpMutexPruningMethod::state_reachability(int int_state, int src_state, const CondensedTransitionSystem &cts, std::vector<bool> &reach) {
    for (int state_j = 0; state_j < cts.num_abstract_states; state_j++) {
        if (!REACH_XY(src_state, state_j)) {
            size_t i;
            for (i = 0; cts.abstract_transitions[i].src != int_state && i < cts.abstract_transitions.size(); ++i);
            for (; cts.abstract_transitions[i].src == int_state && i < cts.abstract_transitions.size(); ++i) {
                if (cts.abstract_transitions[i].target == state_j) {
                    REACH_XY(src_state, state_j) = true;
                    state_reachability(state_j, src_state, cts, reach);
                    break;
                }
            }
        }
    }
}

/*
 * This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
 * calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
 * to be a DAG.
 */
void OpMutexPruningMethod::reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, const int state) {
    // Check whether this state has been visited before, states can always reach themselves
    if (!REACH_XY(state, state)) {
        // Flag that the current state can reach itself
        REACH_XY(state, state) = 1;

        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            reachability(cts, reach, t.target);

            // Copy reachability of the target states to the current state
            for (int j = 0; j < cts.num_abstract_states; ++j) {
                REACH_XY(state, j) += REACH_XY(t.target, j);
            }
        }
    }
}

void OpMutexPruningMethod::reach_print(const CondensedTransitionSystem &cts, const vector<int> &reach) {
    utils::g_log << " ";
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << " " << i;
    }
    utils::g_log << endl;
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << i;
        for (int j = 0; j < cts.num_abstract_states; ++j) {
            utils::g_log << " " << REACH_XY(i,j);
        }
        utils::g_log << endl;
    }
}


static shared_ptr<OpMutexPruningMethod> _parse(OptionParser &parser) {
    parser.document_synopsis(
            "Operator Mutex Pruning",
            "Operator Mutex Pruning");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<OpMutexPruningMethod>(opts);
}

static Plugin<OpMutexPruningMethod> _plugin("op_mutex", _parse);

static options::PluginTypePlugin<OpMutexPruningMethod> _type_plugin(
    "op_mutex",
    "Operator mutex type plugin.");
}

