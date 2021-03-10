#ifndef FAST_DOWNWARD_OPERATOR_MUTEX_H
#define FAST_DOWNWARD_OPERATOR_MUTEX_H

#include "../pruning_method.h"
#include "../merge_and_shrink/factored_transition_system.h"
#include "../merge_and_shrink/transition_system.h"
#include "condensed_transition_system.h"
using namespace std;

using merge_and_shrink::FactoredTransitionSystem;
using merge_and_shrink::Transition;

namespace op_mutex_pruning {
class OpMutexPruningMethod {
private:

public:
    explicit OpMutexPruningMethod(const options::Options &opts);
    FactoredTransitionSystem* run(FactoredTransitionSystem* fts);

    void state_reachability(int int_state, int src_state, const CondensedTransitionSystem &cts, std::vector<bool> &reach);

    void reachability(const CondensedTransitionSystem &cts, vector<int> &reach, const int state);
    static void reach_print(const CondensedTransitionSystem &cts, const vector<int> &reach);

    std::vector<std::pair<int, int>>
    infer_label_mutex_in_condensed_ts(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler);
};

}

#endif
