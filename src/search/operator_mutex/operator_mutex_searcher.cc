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
#include "labeled_transition.h"
#include "ts_to_dot.h"
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
OperatorMutexSearcher::OperatorMutexSearcher(const Options &opts){
    reachability_strategy = opts.get<shared_ptr<ReachabilityStrategy>>("reachability");
    previous_ops_strategy = opts.get<shared_ptr<PreviousOps>>("previous_ops");

    max_ts_size = opts.get<int>("max_ts_size");
    run_on_intermediate = opts.get<bool>("use_intermediate");
    utils::d_log.is_debug = opts.get<bool>("debug");
}

    __attribute__((unused)) bool transition_label_comparison(LabeledTransition t, int l) { return t.label_group < l; }

    __attribute__((unused)) bool transition_comparison(LabeledTransition a, int b) { return a.src < b; }

void OperatorMutexSearcher::run(FactoredTransitionSystem &fts) {
    double start_time = utils::g_timer();
    utils::d_log << "Operator mutex starting iteration " << iteration << endl;
    size_t num_opmutex_before = label_mutexes.size();

    // Iterate over all active indices in the fts
    for (int fts_i : fts) {
        if (num_op_mutex_in_previous_run_of_fts_i.size() <= size_t(fts_i)) {
            num_op_mutex_in_previous_run_of_fts_i.resize(fts_i + 1);
            goto must_run;
        }
        if (size_t(num_op_mutex_in_previous_run_of_fts_i[fts_i]) == label_mutexes.size())
            continue;

        must_run:

        TransitionSystem ts = fts.get_transition_system(fts_i);
        if (ts.get_num_states() <= max_ts_size) {
            size_t before_label_mutexes = label_mutexes.size();
            infer_label_group_mutex_in_ts(fts, fts_i);
//            utils::d_log << "In ts_" << iteration << "_" << fts_i << " of size " << ts.get_num_states() << " found num new op-mutexes " << (label_mutexes.size() - before_label_mutexes)  << " Total op-mutexes: " << label_mutexes.size() << endl;

            num_op_mutex_in_previous_run_of_fts_i[fts_i] = label_mutexes.size();
        }
    }

    utils::g_log << "Operator mutex finished iteration " << iteration << ". Found an additional " << (label_mutexes.size() - num_opmutex_before) << " operator mutexes" << endl << endl;
    iteration++;
    runtime += utils::g_timer() - start_time;
}

void OperatorMutexSearcher::infer_label_group_mutex_in_ts(FactoredTransitionSystem &fts, int fts_index) {
    TransitionSystem ts = fts.get_transition_system(fts_index);
    int initial_state = ts.get_init_state();
    unordered_set<int> goal_states;

    for (int s = 0; s < ts.get_num_states(); s++) {
        if (ts.is_goal_state(s))
            goal_states.insert(s);
    }

    shared_ptr<LabelEquivalenceRelation> ler = ts.get_label_equivalence_relation();
    vector<vector<Transition>> tbg = ts.get_transitions();
    vector<LabeledTransition> labeled_transitions;

    for (int group = 0; group < ler->get_size(); group++) {
        for (Transition t : tbg[group]) {
            labeled_transitions.emplace_back(t.src, t.target, group);
        }
    }

    //tils::g_log << ts_to_dot(labeled_transitions, ts) << endl;

    CondensedTransitionSystem cts = CondensedTransitionSystem(fts_index, labeled_transitions, ts.get_num_states(),
                                                              initial_state, goal_states);

    //utils::g_log << cts_to_dot(cts) << endl;

    if (cts.abstract_transitions.empty()) {
        // This CTS is unusable
        utils::g_log << "The problem has NO transitions!" << endl;
        return;
    }

    // Compute unreachable states based on already known op-mutexes
    previous_ops_strategy->run(cts, ler, label_mutexes);

    if (previous_ops_strategy->will_prune()) {
        vector<vector<Transition>> new_transitions(tbg.size());
        vector<LabeledTransition> abstract_transitions = cts.abstract_transitions;

        for (size_t g = 0; g < tbg.size(); g++) {
            for (Transition &t : tbg[g]) {
                LabeledTransition at(cts.concrete_to_abstract_state[t.src], cts.concrete_to_abstract_state[t.target], g);
                int l = lower_bound(abstract_transitions.begin(), abstract_transitions.end(), at) - abstract_transitions.begin();

                if (abstract_transitions[l] == at) {
                    new_transitions[g].push_back(t);
                }
            }
        }
        fts.set_transitions(fts_index, new_transitions);
    }

    vector<pair<int, int>> label_group_mutexes = infer_label_group_mutex_in_condensed_ts(cts);

    // All operators are implicitly op-mutex with operators that have NO transitions
    auto ts2 = fts.get_transition_system(fts_index);
    auto tbg2 = ts2.get_transitions();
    for (size_t og = 0; og < tbg2.size(); og++) {
        if (tbg2[og].empty()) {
            for (size_t ig = 0; ig < tbg2.size(); ig++) {
                label_group_mutexes.emplace_back(og, ig);
            }
        }
    }

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
#define TRANS_IDX(idx) (cts.abstract_transitions[idx])

/*
* This function computes label mutexes, by checking for a pair of label groups (g1, g2) that all the transitions belonging
* to g1 are transition-mutex with all transitions belonging to g2. Two transitions s_1 -l1> s_2, s_1' -l2> s_2' are transition-
* mutex iff there is no path from s_2' to s_1 and from s_2 to s_1'. This check is done using a reachability matrix.
*/
std::vector<std::pair<int, int>>
OperatorMutexSearcher::infer_label_group_mutex_in_condensed_ts(CondensedTransitionSystem &cts) {
    // Start from the initial state of planning problem
    vector<bool> reach = reachability_strategy->run(cts);

    // Sort transitions on label group
    std::sort(cts.abstract_transitions.begin(), cts.abstract_transitions.end(), label_comparison);

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
    int last_label_group = cts.abstract_transitions.back().label_group;
    while (current_outer_label < last_label_group) { // Do not consider last label
        int to_start = to_end; // End is exclusive

        // Search for the next label, could be binary search, but not really worth it
        for (; TRANS_IDX(to_end).label_group <= current_outer_label &&
               to_end < cts.abstract_transitions.size(); to_end++);

        // Start looking from the next label group up, we only need to check half of the combinations because label
        // mutexes are symmetric
        int current_inner_label = current_outer_label;

        size_t ti = to_start;

        bool is_label_mutex = true;
        for (; ti < cts.abstract_transitions.size(); ti++) {
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
                if (REACH_XY(TRANS_IDX(to).target, TRANS_IDX(ti).src)
                    || REACH_XY(TRANS_IDX(ti).target, TRANS_IDX(to).src)) {
                    is_label_mutex = false;
                    break;
                }
            }
        }
        // Check if the last label we considered is mutex, this is needed because the above for loop exits before we can
        // check the last one.
        if (!cts.abstract_transitions.empty() && is_label_mutex) {
            // Add only one of the two symmetric mutexes (we will add the other later)
            label_group_mutexes.emplace_back(pair<int, int>(current_outer_label, current_inner_label));
        }

        if (to_end >= cts.abstract_transitions.size())
            break;
        current_outer_label = TRANS_IDX(to_end).label_group;
    }

    return label_group_mutexes;
}

void OperatorMutexSearcher::finalize(FactoredTransitionSystem &fts) {
    auto labels = fts.get_labels_fixed();

    utils::g_log << "Number of labels: " << labels->get_size() << endl;
    utils::g_log << "Total number of operator mutexes: " << label_mutexes.size() << endl;
    utils::g_log << "Operator Mutex total time: " << utils::Duration(runtime) << endl;
//    for (OpMutex om : label_mutexes) {
//        utils::g_log << om.label1 << ", " << om.label2 << " : " << labels->get_name(om.label1) << ", "
//                     << labels->get_name(om.label2) << endl;
//    }
}

void add_algo_options_to_parser(OptionParser &parser) {
    parser.add_option<shared_ptr<ReachabilityStrategy>>(
            "reachability",
            "This option is used for determining the strategy used for computing which states are reachable. "
            "The default strategy is 'Goal()'. Other strategies are 'NoGoal()'.",
            "goal");

    parser.add_option<shared_ptr<PreviousOps>>(
            "previous_ops",
            "Use previous operator mutexes to find unreachable states. "
            "The default strategy is 'NoPO()'. Other strategies are 'NaSUSPO()', 'NaSUTPO()', 'NeLUSPO()', 'NeLUTPO()' and 'BDDOLMPO()'.",
            "nopo");

    parser.add_option<int>(
            "max_ts_size",
            "Maximum number of states a transition system can have that will be considered.",
            "5000");

    parser.add_option<bool>(
            "use_intermediate",
            "Whether or not intermediate factors of MAS should be used.",
            "true");

    parser.add_option<bool>(
            "debug",
            "Extra prints that contains extra information and debug details.",
            "false");
}

static shared_ptr<OperatorMutexSearcher> _parse(OptionParser &parser) {
    parser.document_synopsis(
            "Operator Mutex Pruning",
            "Operator Mutex Pruning");
    add_algo_options_to_parser(parser);
    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<OperatorMutexSearcher>(opts);
}

static Plugin<OperatorMutexSearcher> _plugin("op_mutex", _parse);

static shared_ptr<OperatorMutexSearcher> _none_parse(OptionParser &parser) {
    parser.document_synopsis(
            "No Operator Mutex Pruning",
            "This will do NO operator mutex pruning");

    options::Options opts = parser.parse();
    return nullptr;
}

static Plugin<OperatorMutexSearcher> _plugin_none("no_operator_mutex", _none_parse);

static options::PluginTypePlugin<OperatorMutexSearcher> _type_plugin(
        "op_mutex",
        "Operator mutex type plugin.");
}




