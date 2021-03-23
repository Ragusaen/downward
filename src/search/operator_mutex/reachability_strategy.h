//
// Created by slorup on 3/17/21.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#ifndef FAST_DOWNWARD_REACHABILITY_STRATEGY_H
#define FAST_DOWNWARD_REACHABILITY_STRATEGY_H


#include "condensed_transition_system.h"
#include <unordered_set>

namespace reachability {


class ReachabilityStrategy {

public:
    virtual std::vector<int> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) = 0;
};

class GoalReachability : public ReachabilityStrategy {
public:
    std::vector<int> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) override;
private:
    bool reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, std::vector<bool> &has_visited, int state, const std::unordered_set<int> &unreachable_states);

};

class NoGoalReachability : public ReachabilityStrategy {
public:
    std::vector<int> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) override;
private:
    void reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state, const std::unordered_set<int> &unreachable_states);

};


void reach_compare(const CondensedTransitionSystem &cts, const std::unordered_set<int>& unreachable);
void reach_compare_prev(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable, std::unique_ptr<ReachabilityStrategy> reachStrat);
}

#endif //FAST_DOWNWARD_REACHABILITY_STRATEGY_H

#pragma clang diagnostic pop