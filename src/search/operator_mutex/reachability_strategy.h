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
    virtual ~ReachabilityStrategy() = default;
};

class Goal : public ReachabilityStrategy {
public:
    std::vector<bool> run(const CondensedTransitionSystem &cts) override;
    explicit Goal(const options::Options&) { }
    ~Goal() override = default;
private:
    bool reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, std::vector<bool> &has_visited, int state);
};

class NoGoal : public ReachabilityStrategy {
public:
    explicit NoGoal(const options::Options&) { }
    ~NoGoal() override = default;

    std::vector<bool> run(const CondensedTransitionSystem &cts) override;
private:
    void reachability(const CondensedTransitionSystem &cts, std::vector<bool> &reach, int state);
};
}

#endif //FAST_DOWNWARD_REACHABILITY_STRATEGY_H