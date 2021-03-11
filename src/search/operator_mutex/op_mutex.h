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
    struct OpMutex {
        int label1;
        int label2;

        OpMutex(int l1, int l2) : label1(l1), label2(l2) {}
    };
    static inline bool operator==(const OpMutex a, const OpMutex b) {
        return a.label1 == b.label1 && a.label2 == b.label2;
    }
}

namespace std {
    template<> struct hash<op_mutex_pruning::OpMutex>
    {
        std::size_t operator()(const op_mutex_pruning::OpMutex& p) const noexcept
        {
            return p.label1 ^ p.label2 << 16; // Bitshift 16 bits to create least overlap between labels
        }
    };
}

namespace op_mutex_pruning {


class OpMutexPruningMethod {
private:
    unordered_set<OpMutex> label_mutexes;

public:
    explicit OpMutexPruningMethod(const options::Options &opts);

    void state_reachability(int int_state, int src_state, const CondensedTransitionSystem &cts, std::vector<bool> &reach);

    bool reachability(const CondensedTransitionSystem &cts, vector<int> &reach, const int state);
    static void reach_print(const CondensedTransitionSystem &cts, const vector<int> &reach);

    void run(FactoredTransitionSystem &fts);

    void finalize(FactoredTransitionSystem &fts);

    vector<pair<int, int>> infer_label_group_mutex_in_condensed_ts(CondensedTransitionSystem &cts);

    void infer_label_group_mutex_in_ts(TransitionSystem &ts);

    bool reachability_non_goal(const CondensedTransitionSystem &cts, vector<int> &reach, const int state);
};

}



#endif
