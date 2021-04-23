//
// Created by ragusa on 3/26/21.
//

#include "ts_to_dot.h"
#include <algorithm>

string ts_to_dot(vector<LabeledTransition> labeled_transitions, const TransitionSystem &ts) {
    string out = "digraph G {\n";

    // First add markings to initial state and goal states
    for (int s = 0; s < ts.get_size(); s++) {
        if (ts.is_goal_state(s)) {
            out += "    " + to_string(s) + " [shape=doublecircle];\n";
        }
    }
    out += "    " + to_string(ts.get_init_state()) + " [color=red];\n";

    sort(labeled_transitions.begin(), labeled_transitions.end());

    LabeledTransition current_t = LabeledTransition(-1, -1, -1);
    string current_ls;
    bool first = true;
    for (LabeledTransition t : labeled_transitions) {
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

    LabeledTransition current_t = LabeledTransition(-1, -1, -1);
    string current_ls;
    bool first = true;
    for (LabeledTransition t : cts.abstract_transitions) {
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