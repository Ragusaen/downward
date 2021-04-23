//
// Created by ragusa on 4/14/21.
//

#ifndef FAST_DOWNWARD_LABELED_TRANSITION_H
#define FAST_DOWNWARD_LABELED_TRANSITION_H

#include "../merge_and_shrink/transition_system.h"
using namespace merge_and_shrink;

namespace op_mutex {

struct LabeledTransition : Transition {
    int label_group;

    LabeledTransition(int src, int target, int label_group)
            : Transition(src, target), label_group(label_group) {}

    bool operator==(const LabeledTransition &other) const {
        return src == other.src && target == other.target && label_group == other.label_group;
    }

    bool operator!=(const LabeledTransition &other) const {
        return src != other.src || target != other.target || label_group != other.label_group;
    }

    bool operator<(const LabeledTransition &other) const {
        if (src != other.src)
            return src < other.src;
        else if (target != other.target)
            return target < other.target;
        else
            return label_group < other.label_group;
    }

    // Required for "is_sorted_unique" in utilities
    bool operator>=(const LabeledTransition &other) const {
        return !(*this < other);
    }
};

std::string to_string(LabeledTransition t);

static bool label_comparison(LabeledTransition a, LabeledTransition b) {
    if (a.label_group != b.label_group)
        return a.label_group < b.label_group;
    else if (a.src != b.src)
        return a.src < b.src;
    else
        return a.target < b.target;
}
}

namespace std {
template<>
struct hash<op_mutex::LabeledTransition> {
    std::size_t operator()(const op_mutex::LabeledTransition &t) const noexcept {
        return t.src << 16 ^ t.target << 8 ^ t.label_group; // Bitshift to create least overlap between transitions
    }
};
}


#endif //FAST_DOWNWARD_LABELED_TRANSITION_H
