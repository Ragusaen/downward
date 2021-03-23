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

void reach_compare(const CondensedTransitionSystem &cts, const std::unordered_set<int>& unreachable);

class ReachabilityStrategy {

public:
    virtual std::vector<bool> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) = 0;
};

class GoalReachability : public ReachabilityStrategy {
public:
    std::vector<bool> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) override;
private:
    bool reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, int state, const std::unordered_set<int> &unreachable_states);

};

class NoGoalReachability : public ReachabilityStrategy {
public:
    std::vector<bool> run(const CondensedTransitionSystem &cts, const std::unordered_set<int> &unreachable_states) override;
private:
    void reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, int state, const std::unordered_set<int> &unreachable_states);

};
}

#endif //FAST_DOWNWARD_REACHABILITY_STRATEGY_H

#pragma clang diagnostic pop