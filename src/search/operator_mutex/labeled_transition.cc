
#include "labeled_transition.h"

namespace op_mutex {
    __attribute__((unused)) std::string to_string(LabeledTransition t) {
    return "(" + std::to_string(t.src) + " -" + std::to_string(t.label_group) +"> " + std::to_string(t.target) + ")";
}
}

