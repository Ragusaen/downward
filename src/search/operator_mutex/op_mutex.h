#ifndef FAST_DOWNWARD_OPERATOR_MUTEX_H
#define FAST_DOWNWARD_OPERATOR_MUTEX_H

#include "../pruning_method.h"
#include "../merge_and_shrink/factored_transition_system.h"
#include "../merge_and_shrink/transition_system.h"
#include "CondensedTransitionSystem.h"
using namespace std;

using merge_and_shrink::FactoredTransitionSystem;
using merge_and_shrink::Transition;

namespace op_mutex_pruning {
class OpMutexPruningMethod {
private:
    std::vector<std::pair<Transition, Transition>>
    infer_transition_mutex_in_condensed_ts(CondensedTransitionSystem cts);
public:
    explicit OpMutexPruningMethod(const options::Options &opts);
    FactoredTransitionSystem* run(FactoredTransitionSystem* fts);

    void state_reachability(int int_state, int src_state, CondensedTransitionSystem &cts, std::vector<bool> &reach);

    void odums_algorithm(const CondensedTransitionSystem *cts, vector<bool> *reach);

    void reach_neighbors(const CondensedTransitionSystem *cts, vector<bool> *reach, vector<int> *current_states,
                         vector<bool> *hit_states, int state);

    vector<Transition> get_transitions(int src, const CondensedTransitionSystem *cts);
};
}

#endif
