#include "op_mutex.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include "../merge_and_shrink/merge_and_shrink_algorithm.h"
#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "condensed_transition_system.h"
#include "reachability_strategy.h"
#include "previous_ops.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <fstream>

using namespace std;
using options::Bounds;
using options::OptionParser;
using options::Options;
using utils::ExitCode;
using namespace merge_and_shrink;
using namespace dynamic_bitset;

template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig) {
    std::vector<T> ret;
    for (const auto &v: orig)
        ret.insert(ret.end(), v.begin(), v.end());
    return ret;
}



namespace op_mutex {
OpMutexPruningMethod::OpMutexPruningMethod(const Options &opts){
    auto reachability_option = opts.get<ReachabilityOption>("reachability_strategy");
    switch(reachability_option){
        case ReachabilityOption::GOAL_REACHABILITY:
            reachability_strategy = unique_ptr<ReachabilityStrategy>(new GoalReachability());
            break;
        case ReachabilityOption::NO_GOAL_REACHABILITY:
            reachability_strategy = unique_ptr<ReachabilityStrategy>(new NoGoalReachability());
            break;
    }

    auto previous_ops_option = opts.get<PreviousOpsOption>("use_previous_ops");
    switch (previous_ops_option) {
        case PreviousOpsOption::NONE:
            previous_ops_strategy = unique_ptr<PreviousOps>(new NoPreviousOps());
            break;
        case PreviousOpsOption::SIMPLE:
            previous_ops_strategy = unique_ptr<PreviousOps>(new SimplePreviousOps());
            break;
        case PreviousOpsOption::ALL:
            previous_ops_strategy = unique_ptr<PreviousOps>(new AllPreviousOps());
            break;
    }

    max_ts_size = opts.get<int>("max_ts_size");
}

bool transition_label_comparison(Transition t, int l) { return t.label_group < l; }

bool transition_comparison(Transition a, int b) { return a.src < b; }

void OpMutexPruningMethod::run(FactoredTransitionSystem &fts) {
    double start_time = utils::g_timer();
    utils::g_log << "Operator mutex starting iteration " << iteration << endl;
    int num_opmutex_before = label_mutexes.size();

    // Iterate over all active indices in the fts
    for (int fts_i : fts) {
        TransitionSystem ts = fts.get_transition_system(fts_i);
        if (ts.get_num_states() <= max_ts_size) {
            int before_label_mutexes = label_mutexes.size();
            infer_label_group_mutex_in_ts(ts);
            utils::g_log << "In ts_" << iteration << "_" << fts_i << " of size " << ts.get_num_states() << " found num new op-mutexes " << (label_mutexes.size() - before_label_mutexes) << endl;
        }
    }

    utils::g_log << "Operator mutex finished iteration " << iteration << ". Found an additional " << (label_mutexes.size() - num_opmutex_before) << " operator mutexes" << endl << endl;
    iteration++;
    runtime += utils::g_timer() - start_time;
}

void OpMutexPruningMethod::infer_label_group_mutex_in_ts(TransitionSystem &ts) {
    int initial_state = ts.get_init_state();
    unordered_set<int> goal_states;

    for (int s = 0; s < ts.get_num_states(); s++) {
        if (ts.is_goal_state(s))
            goal_states.insert(s);
    }

    shared_ptr<LabelEquivalenceRelation> ler = ts.get_label_equivalence_relation();
    vector<vector<Transition>> tbg = ts.get_transitions();
    vector<Transition> grouped_transitions = std::vector<Transition>();

    for (int group = 0; group < ler->get_size(); group++) {
        for (Transition t : tbg[group]) {
            grouped_transitions.emplace_back(t.src, t.target, group);
        }
    }

    CondensedTransitionSystem cts = CondensedTransitionSystem(grouped_transitions, ts.get_num_states(),
                                                              initial_state, goal_states);

    // Compute unreachable states based on already known op-mutexes
    unordered_set<int> unreachable_states = previous_ops_strategy->run(cts, ler, label_mutexes);

    vector<pair<int, int>> label_group_mutexes = infer_label_group_mutex_in_condensed_ts(cts, unreachable_states);

    // Expand label groups into concrete labels
    for (auto gm : label_group_mutexes) {
        LabelGroup groupA = ler->get_group(gm.first);
        LabelGroup groupB = ler->get_group(gm.second);

        for (int la : groupA) {
            for (int lb : groupB) {
                add_opmutex(la, lb);
            }
        }
    }
}

// These macros are used as shorthands for looking up reach, transitions and converting from abstract to concrete transitions
#define REACH_XY(x, y) (reach[(x) * cts.num_abstract_states + (y)])
#define TRANS_IDX(idx) (cts.concrete_transitions[idx])
#define C2A(state) (cts.concrete_to_abstract_state[state])

/*
* This function computes label mutexes, by checking for a pair of label groups (g1, g2) that all the transitions belonging
* to g1 are transition-mutex with all transitions belonging to g2. Two transitions s_1 -l1> s_2, s_1' -l2> s_2' are transition-
* mutex iff there is no path from s_2' to s_1 and from s_2 to s_1'. This check is done using a reachability matrix.
*/
std::vector<std::pair<int, int>>
OpMutexPruningMethod::infer_label_group_mutex_in_condensed_ts(CondensedTransitionSystem &cts, unordered_set<int> &unreachable_states) {
    // Start from the initial state of planning problem
    vector<bool> reach = reachability_strategy->run(cts, unreachable_states);

    // Sort transitions on label group
    std::sort(cts.concrete_transitions.begin(), cts.concrete_transitions.end(),
              [](Transition a, Transition b) { return a.label_group < b.label_group; });

    // Find label_group mutexes
    vector<pair<int, int>> label_group_mutexes = vector<pair<int, int>>();

    int current_outer_label = 0;
    size_t to_end = 0;

    /*
    * The general idea of this loop is to iterate through all the transitions, but consider them within their label group.
    * The current_outer_label is the label that the outer loop is currently at, there are variable to_start and to_end
    * (to_end is exclusive) that correspond to the outer labels first and last transition in the vector. Then, for each
    * transition, starting from to_end as we only consider one of the two symmetric label pairs, we check if this
    * transition is transition mutex with all of the [to_start, to_end) transitions. When ti points to a new label, we
    * see if there were any non-transition-mutexes for the inner label, and if not we add it to the label mutex vector.
    * We skip all labels with 0 transitions, as these are trivially op-mutex with all other labels, and there is no need
    * to store all of them.
    */
    int last_label_group = cts.concrete_transitions.back().label_group;
    while (current_outer_label < last_label_group) { // Do not consider last label
        int to_start = to_end; // End is exclusive

        // Search for the next label, could be binary search, but not really worth it
        for (; TRANS_IDX(to_end).label_group <= current_outer_label &&
               to_end < cts.concrete_transitions.size(); to_end++);

        // Start looking from the next label group up, we only need to check half of the combinations because label
        // mutexes are symmetric
        int current_inner_label = current_outer_label;

        size_t ti = to_start;

        bool is_label_mutex = true;
        for (; ti < cts.concrete_transitions.size(); ti++) {
            if (TRANS_IDX(ti).label_group > current_inner_label) {
                if (is_label_mutex) {
                    // Add only one of the two symmetric mutexes (we will add the other later)
                    label_group_mutexes.emplace_back(pair<int, int>(current_outer_label, current_inner_label));
                }

                // Start with next label
                current_inner_label = TRANS_IDX(ti).label_group;
                is_label_mutex = true;
            }

            for (size_t to = to_start; to < to_end; to++) {
                // If these two transitions are not mutex, the labels also aren't
                if (REACH_XY(C2A(TRANS_IDX(to).target), C2A(TRANS_IDX(ti).src))
                    || REACH_XY(C2A(TRANS_IDX(ti).target), C2A(TRANS_IDX(to).src))) {
                    is_label_mutex = false;
                    break;
                }
            }
        }
        // Check if the last label we considered is mutex, this is needed because the above for loop exits before we can
        // check the last one.
        if (!cts.concrete_transitions.empty() && is_label_mutex) {
            // Add only one of the two symmetric mutexes (we will add the other later)
            label_group_mutexes.emplace_back(pair<int, int>(current_outer_label, current_inner_label));
        }

        if (to_end >= cts.concrete_transitions.size())
            break;
        current_outer_label = TRANS_IDX(to_end).label_group;
    }

    return label_group_mutexes;
}

void OpMutexPruningMethod::finalize(FactoredTransitionSystem &fts) {
    auto labels = fts.get_labels_fixed();

    utils::g_log << labels->get_size() << endl;
    utils::g_log << "Total number of operator mutexes: " << label_mutexes.size() << endl;
    utils::g_log << "Operator Mutex total time: " << utils::Duration(runtime) << endl;
//    for (OpMutex om : label_mutexes) {
//        utils::g_log << om.label1 << ", " << om.label2 << " : " << labels->get_name(om.label1) << ", "
//                     << labels->get_name(om.label2) << endl;
//    }
}

void add_algo_options_to_parser(OptionParser &parser) {
    vector<string> reachability_options;
    reachability_options.emplace_back("goal");
    reachability_options.emplace_back("no_goal");
    parser.add_enum_option<ReachabilityOption>(
            "reachability_strategy",
            reachability_options,
            "This option is used for determining the strategy used for computing which states are reachable. "
            "The default strategy is 'goal'. Other strategies are 'no_goal'.",
            "goal");

    vector<string> previous_ops_options;
    previous_ops_options.emplace_back("none");
    previous_ops_options.emplace_back("simple");
    previous_ops_options.emplace_back("all");
    parser.add_enum_option<PreviousOpsOption>(
            "use_previous_ops",
            previous_ops_options,
            "Use previous operator mutexes to find unreachable states. "
            "The default strategy is 'none'. Other strategies are 'simple' and 'all'. ",
            "none");

    parser.add_option<int>(
            "max_ts_size",
            "Maximum number of states a transition system can have that will be considered",
            "5000"
            );
}

static shared_ptr<OpMutexPruningMethod> _parse(OptionParser &parser) {
    parser.document_synopsis(
            "Operator Mutex Pruning",
            "Operator Mutex Pruning");
    add_algo_options_to_parser(parser);
    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<OpMutexPruningMethod>(opts);
}

static Plugin<OpMutexPruningMethod> _plugin("op_mutex", _parse);

static shared_ptr<OpMutexPruningMethod> _none_parse(OptionParser &parser) {
    parser.document_synopsis(
            "No Operator Mutex Pruning",
            "This will do NO operator mutex pruning");

    options::Options opts = parser.parse();
    return nullptr;
}

static Plugin<OpMutexPruningMethod> _plugin_none("no_operator_mutex", _none_parse);

static options::PluginTypePlugin<OpMutexPruningMethod> _type_plugin(
        "op_mutex",
        "Operator mutex type plugin.");
}




