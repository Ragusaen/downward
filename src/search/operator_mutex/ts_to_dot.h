//
// Created by ragusa on 3/26/21.
//

#ifndef FAST_DOWNWARD_TS_TO_DOT_H
#define FAST_DOWNWARD_TS_TO_DOT_H

#include <string>
#include "condensed_transition_system.h"

using namespace std;
using namespace op_mutex;

string ts_to_dot(vector<LabeledTransition> labeled_transitions, const TransitionSystem &ts);
string cts_to_dot(const CondensedTransitionSystem &cts);


#endif //FAST_DOWNWARD_TS_TO_DOT_H
