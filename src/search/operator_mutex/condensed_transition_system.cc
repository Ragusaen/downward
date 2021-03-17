#include "condensed_transition_system.h"

#include <utility>
#include <algorithm>
#include <string>
#include <cassert>
#include <vector>

#include "../utils/logging.h"
using namespace std;

std::string to_string(Transition t) {
    return std::to_string(t.src) + " -" + (t.label_group != -1? std::to_string(t.label_group): "") + "> " + std::to_string(t.target);
}

bool finishing_time_pair_comparison(std::pair<int,int> a, std::pair<int,int> b) {
    return a.second > b.second;
}

bool transition_comparison(Transition a, Transition b) {
    if (a.src < b.src)
        return true;
    else if (a.src == b.src)
        return a.target < b.target;
    else
        return false;
}

CondensedTransitionSystem::CondensedTransitionSystem(std::vector<Transition> concrete_transitions,
                                                     int num_concrete_states, int initial_concrete_state,
                                                     const unordered_set<int>& goal_states):
    concrete_transitions(std::move(concrete_transitions)),
    num_abstract_states(0),
    num_concrete_states(num_concrete_states)
{
    abstract_transitions = std::vector<Transition>();
    concrete_to_abstract_state = std::vector<int>(num_concrete_states, -1);
    concrete_to_abstract_transitions = std::vector<int>();

    // Discover strongly connected components
    discover_sccs();

    // Create initial states for the abstract transition system
    initial_abstract_state = concrete_to_abstract_state[initial_concrete_state];

    // Create goal states for the abstract transition system
    for (const int goal_state : goal_states) {
        abstract_goal_states.insert(concrete_to_abstract_state[goal_state]);
    }
}

void CondensedTransitionSystem::discover_sccs() {
    // Depth first search to generate finishing times of each state
    std::vector<std::pair<int,int>> finishing_times = depth_first_search();

    // Mapping concrete states to abstract states
    transpose_depth_first_search(finishing_times);

    // Generate abstract transitions
    for (Transition & ct : concrete_transitions) {
        Transition at = Transition(concrete_to_abstract_state[ct.src], concrete_to_abstract_state[ct.target]);
        abstract_transitions.push_back(at);
    }

    // Sort abstract transitions by source, if equal then by target.
    std::sort(abstract_transitions.begin(), abstract_transitions.end(), transition_comparison);

    // Remove duplicates by copying transitions into a new vector and skipping duplicates
    vector<Transition> abstract_transitions_no_dup = vector<Transition>();
    if (!abstract_transitions.empty()) {
        Transition last = abstract_transitions[0];
        abstract_transitions_no_dup.push_back(last);
        for (size_t i = 1; i < abstract_transitions.size(); ++i) {
            if (abstract_transitions[i] != last) {
                abstract_transitions_no_dup.push_back(abstract_transitions[i]);
                last = abstract_transitions[i];
            }
        }
    }
    abstract_transitions = abstract_transitions_no_dup;

    // Generate mapping of concrete to abstract transitions
    for (Transition & ct : concrete_transitions) {
        auto at = Transition(concrete_to_abstract_state[ct.src], concrete_to_abstract_state[ct.target]);
        for (size_t j = 0; j < abstract_transitions.size(); ++j) {
            if (at.src == abstract_transitions[j].src && at.target == abstract_transitions[j].target) {
                concrete_to_abstract_transitions.push_back(j);
                break;
            }
        }
    }
}

// Performs depth_first_search to find finishing times for each node
std::vector<std::pair<int, int>> CondensedTransitionSystem::depth_first_search() {
    // Store a vector of pairs (state, finishing_time). A pair is needed because this vector is sorted later on
    std::vector<std::pair<int,int>> finishing_times = std::vector<std::pair<int,int>>(num_concrete_states);
    // Store if a state has been visited or not
    std::vector<bool> has_visited = std::vector<bool>(num_concrete_states, false);

    // Sort concrete transitions by source then by target
    std::sort(concrete_transitions.begin(), concrete_transitions.end(), transition_comparison);

    // Visit every state in turn unless it has already been visited
    int time = 0;
    for (int s = 0; s < num_concrete_states; s++) {
        if (!has_visited[s])
            dfs_visit(s, &time, concrete_transitions, &finishing_times, &has_visited);
    }

    return finishing_times;
}

// 'Depth first search visit' The functionality regarding visiting a state during depth first search
void CondensedTransitionSystem::dfs_visit(int s, int* time, const std::vector<Transition>& ts, std::vector<std::pair<int, int>> *finishing_times, std::vector<bool>* has_visited) {
    // Increase timer and set current state to visited
    (*time)++;
    has_visited->at(s) = true;

    // Find the first index i in the vector ts where the source = s
    size_t i = lower_bound(ts.begin(), ts.end(), s, [](Transition t, int s) { return t.src < s; }) - ts.begin();

    // Iterate through transitions while source = s
    // For each transition, visit the target if it has not already been visited
    for (; ts[i].src == s && i < ts.size(); i++) {
        int target = ts[i].target;
        if (!has_visited->at(target))
            dfs_visit(target, time, ts, finishing_times, has_visited);
    }

    // Increase timer and update the finishing time for the current state
    (*time)++;
    finishing_times->at(s) = std::pair<int,int>(s,*time);
}

// Generate a mapping from concrete to abstract states by performing depth first search in the order of decreasing finishing time
void CondensedTransitionSystem::transpose_depth_first_search(std::vector<std::pair<int,int>> finishing_times) {
    // Create a vector with concrete transitions that are "flipped"
    std::vector<Transition> transpose_transitions = std::vector<Transition>();

    // emplace_back creates a 'Transition' and pushes it to the vector without making any intermediate object, thus it is an optimisation
    for (auto t : concrete_transitions) {
        transpose_transitions.emplace_back(t.target, t.src);
    }

    // Sort the transposed transitions by source then by target
    std::sort(transpose_transitions.begin(), transpose_transitions.end(), transition_comparison);

    // Sort vector by decreasing finishing time
    std::sort(finishing_times.begin(), finishing_times.end(), finishing_time_pair_comparison);

    // Vector to keep track off whether a state has been visited or not
    std::vector<bool> has_visited = std::vector<bool>(num_concrete_states, false);
    int current_scc = 0;

    // Visit every state in turn unless it has already been visited
    // Each tree increases the scc counter
    for (std::pair<int,int> v: finishing_times) {
        if (!has_visited[v.first]) {
            tdfs_visit(v.first, &current_scc, transpose_transitions, &has_visited);
            current_scc++;
            num_abstract_states++;
        }
    }
}

void CondensedTransitionSystem::tdfs_visit(int s, int* current_scc, const std::vector<Transition>& ts, std::vector<bool>* has_visited) {
    // Set current state to visited
    has_visited->at(s) = true;

    // Add mapping from current state to current scc
    concrete_to_abstract_state[s] = *current_scc;

    // Find the first index i in the vector ts where the source = s
    size_t i = lower_bound(ts.begin(), ts.end(), s, [](Transition t, int s) { return t.src < s; }) - ts.begin();

    // Iterate through transitions while source = s
    // For each transition, visit the target if it has not already been visited
    for (; ts[i].src == s && i < ts.size(); i++) {
        int target = ts[i].target;
        if (!has_visited->at(target)) {
            tdfs_visit(target, current_scc, ts, has_visited);
        }
    }
}

// Find all abstract transitions with a given source. It is required that the cts' abstract transitions are sorted on source state.
std::vector<Transition> CondensedTransitionSystem::get_abstract_transitions_from_state(int source) const {
    std::vector<Transition> ret = std::vector<Transition>();

    assert(std::is_sorted(abstract_transitions.begin(), abstract_transitions.end(), [](Transition t1, Transition t2) { return t1.src < t2.src; }));

    // Use binary search to find the first index with the src = source
    size_t l = std::lower_bound(abstract_transitions.begin(), abstract_transitions.end(), source,
                              [](Transition t, int s) { return t.src < s; }) - abstract_transitions.begin();

    // Add transitions where src node is equal to parameter 'source'
    for (; abstract_transitions[l].src == source && l < abstract_transitions.size(); l++)
        ret.emplace_back(abstract_transitions[l]);

    return ret;
}
