#ifndef MERGE_AND_SHRINK_MERGE_AND_SHRINK_HEURISTIC_H
#define MERGE_AND_SHRINK_MERGE_AND_SHRINK_HEURISTIC_H

#include "../heuristic.h"
#include "merge_and_shrink_algorithm.h"
#include "../operator_mutex/op_mutex_status_manager.h"

#include <memory>

namespace utils {
enum class Verbosity;
}

namespace merge_and_shrink {
class FactoredTransitionSystem;
class MergeAndShrinkRepresentation;

class MergeAndShrinkHeuristic : public Heuristic {
    const utils::Verbosity verbosity;

    // The final merge-and-shrink representations, storing goal distances.
    std::vector<std::unique_ptr<MergeAndShrinkRepresentation>> mas_representations;

    //op-mutex manager
    op_mutex::OpMutexStatusManager op_mutex_manager;

    const std::shared_ptr<op_mutex::OperatorMutexSearcher> operator_mutex_pruning;

    void extract_factor(FactoredTransitionSystem &fts, int index);
    bool extract_unsolvable_factor(FactoredTransitionSystem &fts);
    void extract_nontrivial_factors(FactoredTransitionSystem &fts);
    void extract_factors(FactoredTransitionSystem &fts);
protected:
    virtual int compute_heuristic(const State &ancestor_state) override;
public:
    explicit MergeAndShrinkHeuristic(const options::Options &opts);

    virtual void get_path_dependent_evaluators(
            std::set<Evaluator *> &evals) override {
        evals.insert(this);
    }

    virtual void notify_initial_state(const State &initial_state) override;
    virtual bool notify_state_transition(const State &parent_state,
                                         OperatorID op_id,
                                         const State &state) override;
    virtual bool dead_ends_are_reliable() const override;
};
}

#endif
