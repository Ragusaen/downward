#include "previous_ops.h"
#include "../utils/logging.h"

#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "condensed_transition_system.h"
#include "reachability_strategy.h"
#include "ts_to_dot.h"
#include "op_mutex2.h"
#include <iostream>
#include <vector>

using namespace std;

namespace previous_ops {
unordered_set<int> NoPreviousOps::run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes){
    return unordered_set<int>();
}

unordered_set<int> SimplePreviousOps::run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes){
    std::vector<int> remaining_parents(cts.num_abstract_states);
    count_parents(cts, remaining_parents, cts.initial_abstract_state);

    std::unordered_set<int> ready_states;

    assert(remaining_parents[cts.initial_abstract_state] == 0);
    ready_states.insert(cts.initial_abstract_state);

    size_t num_label_groups = ler->get_size();
    vector<DynamicBitset<>> state_labels = vector<DynamicBitset<>>(cts.num_abstract_states, DynamicBitset<>(num_label_groups));
    std::vector<bool> has_visited(cts.num_abstract_states, false);

    //utils::g_log << "Num abstract states: " << cts.num_abstract_states << endl;

    while (!ready_states.empty()) {
        int state = *ready_states.begin();
        ready_states.erase(state);

        //utils::g_log << "Current state " << state << endl;

        std::vector<Transition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);
        for (Transition t  : outgoing_transitions) {
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

    unordered_set<int> unreachable_states;

    for (int state = 0; state < cts.num_abstract_states; state++) {
        auto bitset = state_labels[state];
        vector<int> labels;
        for (size_t i = 0; i < num_label_groups; i++) {
            if (bitset[i]) {
                // If this label group has more than 1 label, they can both be used and none of them are therefore necessary
                if (ler->get_group(i).size() == 1) {
                    labels.emplace_back(*ler->get_group(i).begin());
                }
            }
        }

        for (size_t l1 = 0; l1 < labels.size(); l1++) {
            for (size_t l2 = l1 + 1; l2 < labels.size(); l2++) {
                if (label_mutexes.count(OpMutex(labels[l1], labels[l2]))) {
                    unreachable_states.insert(state);
                    goto state_done;
                }
            }
        }
        state_done:;
    }

    return unreachable_states;
}

void SimplePreviousOps::count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state) {
    std::vector<Transition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);

    for (Transition t : outgoing_transitions) {
        if (t.target == state)
            continue;

        if (parents[t.target] == 0) {
            count_parents(cts, parents, t.target);
        }

        parents[t.target]++;
    }
}

unordered_set<int> AllPreviousOps::run(const CondensedTransitionSystem &cts, shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes){
    unordered_set<OpMutex> label_group_mutexes;

    //utils::g_log << cts_to_dot(cts) << endl;

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

    DynamicBitset<> path(ler->get_size());
    DynamicBitset<> fully_reachable(cts.num_abstract_states);

    unreachable_states_dfs(cts, cts.initial_abstract_state, path, fully_reachable, label_group_mutexes);

    unordered_set<int> unreachable_states = unordered_set<int>();
    for (int s = 0; s < cts.num_abstract_states; s++) {
        if(!fully_reachable[s])
            unreachable_states.insert(s);
    }

    utils::g_log << "Found " << unreachable_states.size() << " unreachable states" << endl;

    return unreachable_states;
}

void AllPreviousOps::unreachable_states_dfs(
        const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
        const unordered_set<OpMutex> &label_group_mutexes)
{
    if (cts.abstract_goal_states.count(state))
        fully_reachable.set(state);

    vector<Transition> outgoing_transitions = cts.get_abstract_transitions_from_state(state);

    for(Transition &t : outgoing_transitions) {
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

}