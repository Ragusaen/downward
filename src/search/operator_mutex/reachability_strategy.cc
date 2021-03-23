#include "reachability_strategy.h"
#include "../utils/logging.h"

using namespace std;

namespace reachability {
#define REACH_XY(x, y) (reach[(x) * cts.num_abstract_states + (y)])

vector<int> GoalReachability::run(const CondensedTransitionSystem &cts, const unordered_set<int> &unreachable_states) {
    vector<int> reach(cts.num_abstract_states * cts.num_abstract_states);
    std::vector<bool> has_visited(cts.num_abstract_states);
    reachability(cts, reach, has_visited, cts.initial_abstract_state, unreachable_states);

    return reach;
}

std::vector<int>
NoGoalReachability::run(const CondensedTransitionSystem &cts, const unordered_set<int> &unreachable_states) {
    std::vector<int> reach(cts.num_abstract_states * cts.num_abstract_states);
    reachability(cts, reach, cts.initial_abstract_state, unreachable_states);

    return reach;
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG.
*/
bool GoalReachability::reachability(const CondensedTransitionSystem &cts, vector<int> &reach, vector<bool> &has_visited,
                                    int state, const std::unordered_set<int> &unreachable_states) {
    // A state can trivially reach the goal, if it is a goal state or if it can reach itself. Because Any state that has
    // ANY reachability can reach the goal, and all states that can reach the goal can reach themselves.
    bool can_reach_goal = cts.abstract_goal_states.count(state) || REACH_XY(state, state);

    // Check whether this state has been visited before, states can always reach themselves
    if (!has_visited[state]) {
        has_visited[state] = true;

        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            if (t.target == state || unreachable_states.count(t.target))
                continue;

            bool target_reach_goal = reachability(cts, reach, has_visited, t.target, unreachable_states);

            // We only care about states that can reach a goal state
            if (target_reach_goal) {
                can_reach_goal = true;

                // Copy reachability of the target states to the current state
                for (int j = 0; j < cts.num_abstract_states; ++j) {
                    REACH_XY(state, j) += REACH_XY(t.target, j);
                }
            }

        }

        if (can_reach_goal) {
            REACH_XY(state, state) = 1;
        }
    }

    return can_reach_goal;
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG. This version does not check if the goal is reachable for each state.
*/
void NoGoalReachability::reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state,
                                      const std::unordered_set<int> &unreachable_states) {
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

void reach_print(const CondensedTransitionSystem &cts, const std::vector<int> &reach) {
    utils::g_log << " ";
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << " " << i;
    }
    utils::g_log << std::endl;
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << i;
        for (int j = 0; j < cts.num_abstract_states; ++j) {
            utils::g_log << " " << REACH_XY(i, j);
        }
        utils::g_log << std::endl;
    }
}

void reach_compare(const CondensedTransitionSystem &cts,
                                 const std::unordered_set<int> &unreachable = std::unordered_set<int>()) {
    NoGoalReachability no_goal_r;
    GoalReachability goal_r;

    std::vector<int> no_goal = no_goal_r.run(cts, unreachable);
    std::vector<int> goal = goal_r.run(cts, unreachable);

    utils::g_log << "No goal reach:" << std::endl;
    reach_print(cts, no_goal);

    utils::g_log << "Goal reach:" << std::endl;
    reach_print(cts, goal);
}

void reach_compare_prev(
        const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable,
        unique_ptr<ReachabilityStrategy> reachStrat) {

    vector<int> reach_prev = reachStrat->run(cts, unreachable);
    vector<int> reach_no_prev = reachStrat->run(cts, unordered_set<int>());

//    utils::g_log << "Reach with previous op-mutexes considered" << endl;
//    reach_print(cts, reach_prev);
//    utils::g_log << "Reach with NO previous op-mutexes considered" << endl;
//    reach_print(cts, reach_no_prev);

    for (int i = 0; i < cts.num_abstract_states; i++) {
        for (int j = 0; j < cts.num_abstract_states; j++) {
            std::size_t idx = i * cts.num_abstract_states + j;
            if (reach_prev[idx] == 0
                && reach_no_prev[idx] != 0)
            {
                utils::g_log << "Prev can reach " << i << "->" << j << " but no prev cannot" << endl;
            }

            if (reach_prev[idx] >= 268435456 || reach_no_prev[idx] >= 268435456) {
                utils::g_log << "WARNING " << i << "->" << j << " is about to encounter overflow";
            }
        }
    }

}


}