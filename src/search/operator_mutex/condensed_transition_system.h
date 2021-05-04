//
// Created by ragusa on 2/25/21.
//

#ifndef FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H
#define FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H


#include <vector>
#include <unordered_set>
#include "../merge_and_shrink/transition_system.h"
#include "labeled_transition.h"

using namespace merge_and_shrink;

namespace op_mutex {

class CondensedTransitionSystem {

public:
    CondensedTransitionSystem(int fts_index, std::vector<LabeledTransition> concrete_transitions, int num_concrete_states,
                              int initial_concrete_state, const std::unordered_set<int> &goal_states);

    int fts_index;
    std::vector<LabeledTransition> abstract_transitions;
    std::vector<LabeledTransition> concrete_transitions;

    int num_abstract_states;
    int num_concrete_states;
    std::vector<int> concrete_to_abstract_state;

    int initial_abstract_state;
    std::unordered_set<int> abstract_goal_states;


    std::vector<LabeledTransition> get_abstract_transitions_from_state(int source) const;

private:

    void discover_sccs();

    std::vector<std::pair<int, int>> depth_first_search();

    void dfs_visit(int s, int *time, const std::vector<LabeledTransition> &ts,
                   std::vector<std::pair<int, int>> *finishing_times,
                   std::vector<bool> *has_visited);

    void transpose_depth_first_search(std::vector<std::pair<int, int>> finishing_times);

    void tdfs_visit(int s, int *current_scc, const std::vector<LabeledTransition> &ts, std::vector<bool> *has_visited);
};

}


#endif //FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H
