#ifndef FAST_DOWNWARD_OPERATOR_MUTEX_H
#define FAST_DOWNWARD_OPERATOR_MUTEX_H

#include "../pruning_method.h"
#include "../merge_and_shrink/factored_transition_system.h"
#include "../merge_and_shrink/transition_system.h"
#include "condensed_transition_system.h"
#include "reachability_strategy.h"
#include "op_mutex2.h"
#include "previous_ops.h"
#include "../algorithms/dynamic_bitset.h"

using namespace reachability;
using namespace dynamic_bitset;
using namespace op_mutex2;
using namespace previous_ops;
using namespace std;

using merge_and_shrink::FactoredTransitionSystem;
using merge_and_shrink::Transition;

namespace op_mutex {
enum class ReachabilityOption {
    GOAL_REACHABILITY,
    NO_GOAL_REACHABILITY
};

enum class PreviousOpsOption{
    NONE,
    SIMPLE,
    ALL
};

class OpMutexPruningMethod {
    unique_ptr<ReachabilityStrategy> reachability_strategy;
    unique_ptr<PreviousOps> previous_ops_strategy;

private:
    int max_ts_size;

    int iteration = 0;

    unordered_set<OpMutex> label_mutexes;

    // This adds both symmetric op-mutexes
    void add_opmutex(int label1, int label2) {
        label_mutexes.emplace(label1, label2);
        label_mutexes.emplace(label2, label1);
    }

public:
    explicit OpMutexPruningMethod(const options::Options &opts);

    void run(FactoredTransitionSystem &fts);

    void finalize(FactoredTransitionSystem &fts);

    void infer_label_group_mutex_in_ts(TransitionSystem &ts);

    double runtime = 0.0;

        vector<pair<int, int>>
    infer_label_group_mutex_in_condensed_ts(CondensedTransitionSystem &cts, unordered_set<int> &unreachable_states);
};

}
#endif
