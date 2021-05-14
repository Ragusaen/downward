#ifndef FAST_DOWNWARD_OPERATOR_MUTEX_H
#define FAST_DOWNWARD_OPERATOR_MUTEX_H

#include "../pruning_method.h"
#include "../merge_and_shrink/factored_transition_system.h"
#include "../merge_and_shrink/transition_system.h"
#include "condensed_transition_system.h"
#include "reachability_strategy.h"
#include "previous_ops.h"
#include "../algorithms/dynamic_bitset.h"

using namespace dynamic_bitset;
using namespace std;

using merge_and_shrink::FactoredTransitionSystem;
using merge_and_shrink::Transition;

namespace op_mutex {
class OperatorMutexSearcher {
    shared_ptr<ReachabilityStrategy> reachability_strategy;
    shared_ptr<PreviousOps> previous_ops_strategy;

private:
    int max_ts_size;

    int iteration = 0;

    unordered_set<OpMutex> label_mutexes;

    vector<int> num_op_mutex_in_previous_run_of_fts_i = vector<int>(1, 0);

    // This adds both symmetric op-mutexes
    void add_opmutex(int label1, int label2) {
        if (label1 != label2)
            label_mutexes.emplace(label1, label2);
    }

public:
    explicit OperatorMutexSearcher(const options::Options &opts);

    void run(FactoredTransitionSystem &fts);

    void finalize(FactoredTransitionSystem &fts);

    void infer_label_group_mutex_in_ts(FactoredTransitionSystem &fts, int fts_index);

    double runtime = 0.0;

    vector<pair<int, int>> infer_label_group_mutex_in_condensed_ts(CondensedTransitionSystem &cts);

    unordered_set<OpMutex> get_label_mutexes(){
        return label_mutexes;
    }

    bool run_on_intermediate = true;
};

}
#endif

