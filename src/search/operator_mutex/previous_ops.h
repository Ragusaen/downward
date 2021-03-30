#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#ifndef FAST_DOWNWARD_PREVIOUS_OPS_H
#define FAST_DOWNWARD_PREVIOUS_OPS_H

#include <unordered_set>
#include "condensed_transition_system.h"
#include "op_mutex2.h"
#include "../algorithms/dynamic_bitset.h"

using namespace dynamic_bitset;
using namespace op_mutex2;
using namespace std;

namespace previous_ops {
class PreviousOps {

public:
    virtual std::unordered_set<int> run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) = 0;
};

class NoPreviousOps : public PreviousOps {
public:
    std::unordered_set<int> run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
};

class SimplePreviousOps : public PreviousOps {
public:
    std::unordered_set<int> run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
private:
    void count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state);
};

class AllPreviousOps : public PreviousOps {
public:
    std::unordered_set<int> run(const CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
private:
    void unreachable_states_dfs(
            const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
            const unordered_set<OpMutex> &label_group_mutex);
};
}



#endif //FAST_DOWNWARD_PREVIOUS_OPS_H
