#include "reachability_strategy.h"
#include "../utils/logging.h"

namespace reachability {
#define REACH_XY(x, y) (reach[(x) * cts.num_abstract_states + (y)])

void GoalReachability::run(const CondensedTransitionSystem &cts, std::vector<int> &reach, std::unordered_set<int> &unreachable_states) {
    reachability(cts, reach, cts.initial_abstract_state, unreachable_states);
}

void NoGoalReachability::run(const CondensedTransitionSystem &cts, std::vector<int> &reach, std::unordered_set<int> &unreachable_states) {
    reachability(cts, reach, cts.initial_abstract_state, unreachable_states);
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG.
*/
bool GoalReachability::reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state, std::unordered_set<int> &unreachable_states) {
    bool can_reach_goal = cts.abstract_goal_states.count(state) == 1;

    // Check whether this state has been visited before, states can always reach themselves
    if (!REACH_XY(state, state)) {
        // Flag that the current state can reach itself
        REACH_XY(state, state) = 1;

        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            if (t.target == state || unreachable_states.count(t.target))
                continue;

            bool target_reach_goal = reachability(cts, reach, t.target, unreachable_states);

            // We only care about states that can reach a goal state
            if (target_reach_goal) {
                can_reach_goal = true;

                // Copy reachability of the target states to the current state
                for (int j = 0; j < cts.num_abstract_states; ++j) {
                    REACH_XY(state, j) += REACH_XY(t.target, j);
                }
            }

        }
    }
    return can_reach_goal;
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG. This version does not check if the goal is reachable for each state.
*/
void NoGoalReachability::reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state, std::unordered_set<int> &unreachable_states) {
    // Check whether this state has been visited before, states can always reach themselves
    if (!REACH_XY(state, state)) {
        // Flag that the current state can reach itself
        REACH_XY(state, state) = 1;

        // Get outgoing transitions
        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            if (t.target == state || unreachable_states.count(t.target))
                continue;

            reachability(cts, reach, t.target, unreachable_states);

            // Copy reachability of the target states to the current state
            for (int j = 0; j < cts.num_abstract_states; ++j) {
                REACH_XY(state, j) += REACH_XY(t.target, j);
            }
        }
    }
}
}



