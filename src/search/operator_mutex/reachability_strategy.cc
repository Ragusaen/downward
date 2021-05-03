#include "reachability_strategy.h"
#include "../utils/logging.h"
#include "../option_parser.h"

#include "op_mutex.h"



using namespace std;

namespace op_mutex {
#define REACH_XY(x, y) (reach[(x) * cts.num_abstract_states + (y)])

vector<bool> Goal::run(const CondensedTransitionSystem &cts) {
    vector<bool> reach(cts.num_abstract_states * cts.num_abstract_states);
    std::vector<bool> has_visited(cts.num_abstract_states);
    reachability(cts, reach, has_visited, cts.initial_abstract_state);

    return reach;
}

std::vector<bool> NoGoal::run(const CondensedTransitionSystem &cts) {
    std::vector<bool> reach(cts.num_abstract_states * cts.num_abstract_states);

    reachability(cts, reach, cts.initial_abstract_state);

    return reach;
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG.
*/
bool Goal::reachability(const CondensedTransitionSystem &cts, vector<bool> &reach, vector<bool> &has_visited,
                        int state) {
    // A state can trivially reach the goal, if it is a goal state or if it can reach itself. Because Any state that has
    // ANY reachability can reach the goal, and all states that can reach the goal can reach themselves.
    bool can_reach_goal = cts.abstract_goal_states.count(state) || REACH_XY(state, state);

    // Check whether this state has been visited before, states can always reach themselves
    if (!has_visited[state]) {
        has_visited[state] = true;

        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            if (t.target == state)
                continue;

            bool target_reach_goal = reachability(cts, reach, has_visited, t.target);

            // We only care about states that can reach a goal state
            if (target_reach_goal) {
                can_reach_goal = true;

                // Copy reachability of the target states to the current state
                for (int j = 0; j < cts.num_abstract_states; ++j) {
                    if(REACH_XY(t.target, j))
                        REACH_XY(state, j) = REACH_XY(t.target, j);
                }
            }

        }

        if (can_reach_goal) {
            REACH_XY(state, state) = true;
        }
    }

    return can_reach_goal;
}

/*
* This function computes the reach between abstract states in the CondensedTransitionSystem. It does so by recursively
* calling itself on its neighbours, and each state then inherits its neighbours reachable states. The cts is assumed to
* to be a DAG. This version does not check if the goal is reachable for each state.
*/
void NoGoal::reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, int state) {
    // Check whether this state has been visited before, states can always reach themselves
    if (!REACH_XY(state, state)) {
        // Flag that the current state can reach itself
        REACH_XY(state, state) = true;

        // Get outgoing transitions
        auto outgoing_transitions = cts.get_abstract_transitions_from_state(state);

        // Compute reachability of all neighboring states
        for (auto t : outgoing_transitions) {
            if (t.target == state)
                continue;

            reachability(cts, reach, t.target);

            // Copy reachability of the target states to the current state
            for (int j = 0; j < cts.num_abstract_states; ++j) {
                if(REACH_XY(t.target, j))
                    REACH_XY(state, j) = REACH_XY(t.target, j);
            }
        }
    }
}

shared_ptr<Goal> _parse_goal(OptionParser &parser) {
    parser.document_synopsis(
            "Goal reachability strategy",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<Goal>(opts);
}

static Plugin<ReachabilityStrategy> _plugin_goal("goal", _parse_goal);

shared_ptr<NoGoal> _parse_nogoal(OptionParser &parser) {
    parser.document_synopsis(
            "NoGoal reachability strategy",
            "Reachability strategy");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<NoGoal>(opts);
}

static Plugin<ReachabilityStrategy> _plugin_no_goal("no_goal", _parse_nogoal);

static PluginTypePlugin<ReachabilityStrategy> _type_plugin("ReachabilityStrategy", "This describes the strategy for computing reachability between states.");
}
