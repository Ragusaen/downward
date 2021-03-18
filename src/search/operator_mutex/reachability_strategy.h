//
// Created by slorup on 3/17/21.
//

#ifndef FAST_DOWNWARD_REACHABILITY_STRATEGY_H
#define FAST_DOWNWARD_REACHABILITY_STRATEGY_H


#include "condensed_transition_system.h"

namespace reachability {
class ReachabilityStrategy {
public:
    virtual void run(const CondensedTransitionSystem &cts, std::vector<int> &reach) = 0;
};

class GoalReachability : public ReachabilityStrategy {
public:
    void run(const CondensedTransitionSystem &cts, std::vector<int> &reach) override;
private:
    bool reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state);
};

class NoGoalReachability : public ReachabilityStrategy {
public:
    void run(const CondensedTransitionSystem &cts, std::vector<int> &reach) override;
private:
    void reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach, int state);
};
}

#endif //FAST_DOWNWARD_REACHABILITY_STRATEGY_H
