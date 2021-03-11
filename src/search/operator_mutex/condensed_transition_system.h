//
// Created by ragusa on 2/25/21.
//

#ifndef FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H
#define FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H


#include <vector>
#include <unordered_set>
#include "../merge_and_shrink/transition_system.h"

using namespace merge_and_shrink;

std::string to_string(Transition);

class CondensedTransitionSystem {

public:
    CondensedTransitionSystem(std::vector<Transition> concrete_transitions, int num_concrete_states,
                              int initial_concrete_state, const std::unordered_set<int>& goal_states);

    std::vector<Transition> abstract_transitions;
    std::vector<Transition> concrete_transitions;
    std::vector<int> concrete_to_abstract_transitions;

    int num_abstract_states;
    int num_concrete_states;
    std::vector<int> concrete_to_abstract_state;

    int initial_abstract_state;
    std::unordered_set<int> abstract_goal_states;

    Transition lookup_concrete(std::vector<Transition>::iterator t);

    std::vector<Transition> get_abstract_transitions_from_state(int source) const;

private:

    void discover_sccs();

    std::vector<std::pair<int, int>> depth_first_search();

    void dfs_visit(int s, int *time, const std::vector<Transition> &ts, std::vector<std::pair<int, int>> *finishing_times,
                   std::vector<bool> *has_visited);

    void transpose_depth_first_search(std::vector<std::pair<int, int>> finishing_times);

    void tdfs_visit(int s, int *current_scc, const std::vector<Transition> &ts, std::vector<bool> *has_visited);
};


#endif //FAST_DOWNWARD_CONDENSED_TRANSITION_SYSTEM_H
