//
// Created by ragusa on 3/26/21.
//

#include "ts_to_dot.h"
#include <algorithm>

string ts_to_dot(const TransitionSystem &ts) {
    string out = "digraph G {\n";

    // First add markings to initial state and goal states
    for (int s = 0; s < ts.get_size(); s++) {
        if (ts.is_goal_state(s)) {
            out += "    " + to_string(s) + " [shape=doublecircle];\n";
        }
    }
    out += "    " + to_string(ts.get_init_state()) + " [color=red];\n";

    vector<Transition> grouped_transitions;
    int group_id = 0;
    for (auto group : ts) {
        for (Transition t : group.transitions) {
            grouped_transitions.emplace_back(t.src, t.target, group_id);
        }
        group_id++;
    }

    sort(grouped_transitions.begin(), grouped_transitions.end(), [](Transition a, Transition b) { if (a.src == b.src) return a.target < b.target; else return a.src < b.src;});

    Transition current_t = Transition(-1, -1, -1);
    string current_ls = "";
    bool first = true;
    for (Transition t : grouped_transitions) {
        if (current_t.src != t.src || current_t.target != t.target) {
            if (!first)
                out += "    " + to_string(current_t.src) + " -> " + to_string(current_t.target) + " [label=\"" + current_ls + "\"];\n";
            current_ls = to_string(t.label_group);
        } else {
            current_ls += "," + to_string(t.label_group);
        }
        first = false;
        current_t = t;
    }
    out += "    " + to_string(current_t.src) + " -> " + to_string(current_t.target) + " [label=\"" + current_ls + "\"];\n";


    out += "}";

    return out;
}

string cts_to_dot(const CondensedTransitionSystem &cts) {
    string out = "digraph G {\n";

    // First add markings to initial state and goal states
    for (int g : cts.abstract_goal_states) {
        out += "    " + to_string(g) + " [shape=doublecircle];\n";
    }
    out += "    " + to_string(cts.initial_abstract_state) + " [color=red];\n";

    Transition current_t = Transition(-1, -1, -1);
    string current_ls = "";
    bool first = true;
    for (Transition t : cts.abstract_transitions) {
        if (current_t.src != t.src || current_t.target != t.target) {
            if (!first)
                out += "    " + to_string(current_t.src) + " -> " + to_string(current_t.target) + " [label=\"" + current_ls + "\"];\n";
            current_ls = to_string(t.label_group);
        } else {
            current_ls += "," + to_string(t.label_group);
        }
        first = false;
        current_t = t;
    }
    out += "    " + to_string(current_t.src) + " -> " + to_string(current_t.target) + " [label=\"" + current_ls + "\"];\n";

    out += "}";

    return out;
}