#ifndef FAST_DOWNWARD_PREVIOUS_OPS_H
#define FAST_DOWNWARD_PREVIOUS_OPS_H

#include <unordered_set>
#include "condensed_transition_system.h"
#include "../algorithms/dynamic_bitset.h"
#include "op_mutex.h"
#include "../option_parser.h"
#include "../plugin.h"
#include "cuddObj.hh"
#include "bdd_utils.h"

using namespace dynamic_bitset;
using namespace std;

namespace op_mutex {
class PreviousOps {

public:
    virtual void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) = 0;
    virtual bool will_prune() {
        return false;
    }
    virtual ~PreviousOps() = default;
protected:
    void count_parents(const CondensedTransitionSystem &cts, std::vector<int> &parents, int state);

    static unordered_set<OpMutex> get_label_group_mutexes(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes);
    static vector<OpMutex> get_label_group_mutexes_vector(const std::shared_ptr<LabelEquivalenceRelation> &ler, unordered_set<OpMutex> &label_mutexes);
};

class NoPO : public PreviousOps {
public:
    void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;
    explicit NoPO(const options::Options&) { }
    ~NoPO() override = default;
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
public:
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
    explicit NaSUTPO(const options::Options&) { }
    ~NaSUTPO() override = default;
private:
    void usable_transitions_dfs(const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path,
                                 unordered_set<LabeledTransition> &usable_transitions, const unordered_set<OpMutex> &label_group_mutexes);
};

class NeLUTPO : public UnreachableTransitionsPreviousOps {
public:
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
    explicit NeLUTPO(const options::Options&){ }
    ~NeLUTPO() override = default;
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
public:
    explicit NeLUSPO(const options::Options&) { }
    ~NeLUSPO() override = default;
protected:
    DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
};

class NaSUSPO : public UnreachableStatesPreviousOps {
public:
    explicit NaSUSPO(const options::Options&) { }
    ~NaSUSPO() override = default;
protected:
    DynamicBitset<> find_unreachable_states(CondensedTransitionSystem &cts, const unordered_set<OpMutex> &label_group_mutexes, int num_label_groups) override;
private:
    void unreachable_states_dfs(
            const CondensedTransitionSystem &cts, int state, DynamicBitset<> &path, DynamicBitset<> &fully_reachable,
            const unordered_set<OpMutex> &label_group_mutexes);
};

typedef vector<BitBDD> BitBDDSet;
typedef vector<BitBDDSet> StateBDDs;

class BDDOLMPO : public PreviousOps {

public:
    explicit BDDOLMPO(const options::Options& opts){
        max_bdd_size = opts.get<int>("max_bdd_size");
        max_bdd_time = opts.get<int>("max_bdd_time");


    }
    ~BDDOLMPO() override = default;

    void run(CondensedTransitionSystem &cts, std::shared_ptr<LabelEquivalenceRelation> ler, unordered_set<OpMutex> &label_mutexes) override;

    bool will_prune() override {
        return true;
    }

private:
    int max_bdd_size;
    int max_bdd_time;

    // Vector of bdds for each state for each fts
    vector<StateBDDs> fts_state_bdd;
    //Cudd bdd_manager;

protected :
    vector<LabeledTransition> find_usable_transitions(CondensedTransitionSystem &cts, vector<OpMutex> &label_group_mutexes, int num_label_groups);

    static void SetOverApprox(vector<BitBDD>& bit_bdd, int numVars, int threshold = 10, bool safe = true, double quality = 1.0);
    static void SetUnderApprox(vector<BitBDD>& bit_bdd, int numVars, int threshold = 0, bool safe = true, double quality = 1.0);

};
}

#endif //FAST_DOWNWARD_PREVIOUS_OPS_H
