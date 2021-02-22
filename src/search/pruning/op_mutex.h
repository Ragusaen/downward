#ifndef FAST_DOWNWARD_OPERATOR_MUTEX_H
#define FAST_DOWNWARD_OPERATOR_MUTEX_H

#include "../pruning_method.h"
#include "../merge_and_shrink/factored_transition_system.h"

using merge_and_shrink::FactoredTransitionSystem;

namespace op_mutex_pruning {
    class OpMutexPruningMethod {

        const int size;
    public:
        explicit OpMutexPruningMethod(const options::Options &opts);
        FactoredTransitionSystem* run(FactoredTransitionSystem* fts);
    };
}

#endif
