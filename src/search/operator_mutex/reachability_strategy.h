#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#ifndef FAST_DOWNWARD_REACHABILITY_STRATEGY_H
#define FAST_DOWNWARD_REACHABILITY_STRATEGY_H


#include "condensed_transition_system.h"
#include "../option_parser.h"
#include "../plugin.h"
#include <unordered_set>

namespace op_mutex {
class ReachabilityStrategy {

public:
    virtual std::vector<bool> run(const CondensedTransitionSystem &cts) = 0;
};

class Goal : public ReachabilityStrategy {
public:
    std::vector<bool> run(const CondensedTransitionSystem &cts) override;
    Goal(options::Options opts) {

    }
private:
    bool reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, std::vector<bool> &has_visited, int state);

};

class NoGoal : public ReachabilityStrategy {
public:
    std::vector<bool> run(const CondensedTransitionSystem &cts) override;

    NoGoal(options::Options opts) {

    }
private:
    void reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, int state);

};

void reach_print(const CondensedTransitionSystem &cts, const std::vector<bool> &reach);
void reach_compare(const CondensedTransitionSystem &cts);
void reach_compare_prev(const CondensedTransitionSystem &cts, std::unique_ptr<ReachabilityStrategy> reachStrat);
}

#endif //FAST_DOWNWARD_REACHABILITY_STRATEGY_H

#pragma clang diagnostic pop