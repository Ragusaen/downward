#include "condensed_transition_system.h"

#include <utility>
#include <algorithm>
#include <string>

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
                                                     int num_concrete_states, int initial_concrete_state):
    concrete_transitions(std::move(concrete_transitions)),
    num_abstract_states(0),
    num_concrete_states(num_concrete_states)
{
    abstract_transitions = std::vector<Transition>();
    concrete_to_abstract_state = std::vector<int>(num_concrete_states, -1);
    concrete_to_abstract_transitions = std::vector<int>();

    discover_sccs();

    initial_abstract_state = concrete_to_abstract_state[initial_concrete_state];
}

std::vector<std::pair<int, int>> CondensedTransitionSystem::depth_first_search() {
    std::vector<std::pair<int,int>> finishing_times = std::vector<std::pair<int,int>>(num_concrete_states);
    std::vector<bool> has_visited = std::vector<bool>(num_concrete_states, false);

    std::sort(concrete_transitions.begin(), concrete_transitions.end(), transition_comparison);

    int time = 0;
    for (int s = 0; s < num_concrete_states; s++) {
        if (!has_visited[s])
            dfs_visit(s, &time, concrete_transitions, &finishing_times, &has_visited);
    }

    return finishing_times;
}



void CondensedTransitionSystem::transpose_depth_first_search(std::vector<std::pair<int,int>> finishing_times) {
    std::vector<Transition> transpose_transitions = std::vector<Transition>();
    for (size_t i = 0; i < concrete_transitions.size(); i++) {
        Transition t = concrete_transitions[i];
        transpose_transitions.push_back(Transition(t.target, t.src));
    }
    std::sort(transpose_transitions.begin(), transpose_transitions.end(), transition_comparison);

    std::sort(finishing_times.begin(), finishing_times.end(), finishing_time_pair_comparison);

    std::vector<bool> has_visited = std::vector<bool>(num_concrete_states, false);
    int current_scc = 0;

    for (std::pair<int,int> v: finishing_times) {
        if (!has_visited[v.first]) {
            tdfs_visit(v.first, &current_scc, transpose_transitions, &has_visited);
            current_scc++;
            num_abstract_states++;
        }

    }
}

void CondensedTransitionSystem::discover_sccs() {
    std::vector<std::pair<int,int>> finishing_times = depth_first_search();

    transpose_depth_first_search(finishing_times);

    for (Transition & ct : concrete_transitions) {
        Transition at = Transition(concrete_to_abstract_state[ct.src], concrete_to_abstract_state[ct.target]);
        abstract_transitions.push_back(at);
    }

    std::sort(abstract_transitions.begin(), abstract_transitions.end(), transition_comparison);

    for (size_t i = 1; i < abstract_transitions.size(); ++i) {
        if (abstract_transitions[i].src == abstract_transitions[i - 1].src
                && abstract_transitions[i].target == abstract_transitions[i - 1].target) {
            abstract_transitions.erase(abstract_transitions.begin() + i);
            i--;
        }
    }

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

void CondensedTransitionSystem::dfs_visit(int s, int* time, const std::vector<Transition>& ts, std::vector<std::pair<int, int>> *finishing_times, std::vector<bool>* has_visited) {
    (*time)++;
    has_visited->at(s) = true;

    size_t i;
    for (i = 0; ts[i].src != s && i < ts.size(); i++);
    for (; ts[i].src == s && i < ts.size(); i++) {
        int target = ts[i].target;
        if (!has_visited->at(target))
            dfs_visit(target, time, ts, finishing_times, has_visited);
    }

    (*time)++;
    finishing_times->at(s) = std::pair<int,int>(s,*time);
}

void CondensedTransitionSystem::tdfs_visit(int s, int* current_scc, const std::vector<Transition>& ts, std::vector<bool>* has_visited) {
    has_visited->at(s) = true;

    concrete_to_abstract_state[s] = *current_scc;

    size_t i;
    for (i = 0; ts[i].src != s && i < ts.size(); i++);
    for (; ts[i].src == s && i < ts.size(); i++) {
        int target = ts[i].target;
        if (!has_visited->at(target)) {
            tdfs_visit(target, current_scc, ts, has_visited);
        }
    }
}

Transition CondensedTransitionSystem::lookup_concrete(std::vector<Transition>::iterator t) {
    return abstract_transitions[concrete_to_abstract_transitions[t - concrete_transitions.begin()]];
}

std::vector<Transition> CondensedTransitionSystem::get_abstract_transitions_from_state(int source) const {
    std::vector<Transition> ret = std::vector<Transition>();

    int l = std::lower_bound(abstract_transitions.begin(), abstract_transitions.end(), source,
                              [](Transition t, int s) { return t.src < s; }) - abstract_transitions.begin();

    for (; abstract_transitions[l].src == source && l < abstract_transitions.size(); l++)
        ret.emplace_back(abstract_transitions[l]);

    return ret;
}
