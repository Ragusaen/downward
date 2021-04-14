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
    virtual void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) = 0;
    virtual bool will_prune() {
        return false;
    }
protected:
    static unordered_set<OpMutex> get_label_group_mutexes(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes);
};

class NoPO : public PreviousOps {
public:
    void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
};

class UnreachableTransitionsPreviousOps : public PreviousOps {
    void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
    bool will_prune() override {
        return true;
    }
protected:
    virtual vector<Transition> find_useable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) = 0;
};

class NaSUTPO : public UnreachableTransitionsPreviousOps {

protected:
    vector<Transition> find_useable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;

private:
    void useable_transitions_dfs(const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path,
                                 unordered_set<Transition> &usable_transitions, const unordered_set<OpMutex> &label_group_mutexes);
};

class UnreachableStatesPreviousOps : public PreviousOps {
    void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
    bool will_prune() override {
        return true;
    }
protected:
    virtual DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) = 0;
};

class NeLUSPO : public UnreachableStatesPreviousOps {
protected:
    DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
private:
    void count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state);
};

class NaSUSPO : public UnreachableStatesPreviousOps {
protected:
    DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
private:
    void unreachable_states_dfs(
            const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
            const unordered_set<OpMutex> &label_group_mutexes);
};

}



#endif //FAST_DOWNWARD_PREVIOUS_OPS_H
