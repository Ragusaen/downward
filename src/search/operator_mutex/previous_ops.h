#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#ifndef FAST_DOWNWARD_PREVIOUS_OPS_H
#define FAST_DOWNWARD_PREVIOUS_OPS_H

#include <unordered_set>
#include "condensed_transition_system.h"
#include "../algorithms/dynamic_bitset.h"
#include "op_mutex.h"

using namespace dynamic_bitset;
using namespace std;

namespace op_mutex {
class PreviousOps {

public:
    virtual void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) = 0;
    virtual bool will_prune() {
        return false;
    }
protected:
    void count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state);

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
    virtual vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) = 0;
};

class NaSUTPO : public UnreachableTransitionsPreviousOps {

protected:
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;

private:
    void usable_transitions_dfs(const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path,
                                 unordered_set<LabeledTransition> &usable_transitions, const unordered_set<OpMutex> &label_group_mutexes);
};

class NeLUTPO : public UnreachableTransitionsPreviousOps {
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;

private:
    static bool is_usable(const DynamicBitset<> &label_landmarks, LabeledTransition transition, const unordered_set<OpMutex> &label_group_mutexes);
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
};

class NaSUSPO : public UnreachableStatesPreviousOps {
protected:
    DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
private:
    void unreachable_states_dfs(
            const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
            const unordered_set<OpMutex> &label_group_mutexes);
};

class BDDDOLMPO : public UnreachableTransitionsPreviousOps {
protected :
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;

};

}



#endif //FAST_DOWNWARD_PREVIOUS_OPS_H