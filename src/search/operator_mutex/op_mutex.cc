#include "op_mutex.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include "../merge_and_shrink/merge_and_shrink_algorithm.h"
#include "../merge_and_shrink/transition_system.h"
#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "condensed_transition_system.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using options::Bounds;
using options::OptionParser;
using options::Options;
using utils::ExitCode;
using namespace merge_and_shrink;

template<typename T>
std::vector<T> flatten(const std::vector<std::vector<T>> &orig)
{
    std::vector<T> ret;
    for(const auto &v: orig)
        ret.insert(ret.end(), v.begin(), v.end());
    return ret;
}

namespace op_mutex_pruning {
OpMutexPruningMethod::OpMutexPruningMethod(const Options &opts)
{}

FactoredTransitionSystem* OpMutexPruningMethod::run(FactoredTransitionSystem* fts, TaskProxy &tp) {
    utils::g_log << "Operator mutex running" << endl;

    TransitionSystem ts = fts->get_transition_system(fts->get_size() - 1);

    shared_ptr<LabelEquivalenceRelation> ler = ts.get_label_equivalence_relation();
    vector<vector<Transition>> tbg = ts.get_transitions();
    vector<Transition> labeled_transitions = std::vector<Transition>();

    for (int group = 0; group < ler->get_size(); group++) {
        auto labels = ler->get_group(group);

        for (int label : labels) {
            for (Transition t : tbg[group]) {
                labeled_transitions.emplace_back(Transition(t.src, t.target, label));
            }
        }
    }


    std::vector<Transition> k_ts = std::vector<Transition>();
    /*
    k_ts.push_back(Transition(0, 1));
    k_ts.push_back(Transition(0, 4));
    k_ts.push_back(Transition(1, 2));
    k_ts.push_back(Transition(2, 3));
    k_ts.push_back(Transition(4, 5));
    k_ts.push_back(Transition(4, 9));
    k_ts.push_back(Transition(5, 6));
    k_ts.push_back(Transition(6, 2));
    k_ts.push_back(Transition(6, 7));
    k_ts.push_back(Transition(7, 8));
    k_ts.push_back(Transition(9, 10));
    k_ts.push_back(Transition(10, 11));
    */


    CondensedTransitionSystem cts = CondensedTransitionSystem(labeled_transitions, ts.get_num_states());

    utils::g_log << "Abstract transitions" << std::endl;
    if (cts.abstract_transitions.size() > 50) {
        utils::g_log << "Found " << cts.abstract_transitions.size() << "!" << std::endl;
    } else {
        for (Transition at : cts.abstract_transitions) {
            utils::g_log << to_string(at) << std::endl;
        }
    }

    shared_ptr<Labels> labels = fts->get_labels_fixed();
    auto label_mutexes = infer_label_mutex_in_condensed_ts(cts, labels->get_size());


    utils::g_log << "Found " << label_mutexes.size() << " operator mutexes!" << std::endl;
    if (label_mutexes.size() < 50) {
        for (auto mutex : label_mutexes) {
            utils::g_log << mutex.first << " is operator mutex with " << mutex.second << "\t opnames: " << labels->get_name(mutex.first) << ", " << labels->get_name(mutex.second) << std::endl;
        }
    }

    utils::g_log << "Operator mutex done" << endl << endl;
    return fts;
}

bool transition_label_comparison(Transition t, int l) { return t.label < l; }

#define REACH_XY(x, y) (x * cts.num_abstract_states + y)

std::vector<std::pair<int, int>>
OpMutexPruningMethod::infer_label_mutex_in_condensed_ts(CondensedTransitionSystem cts, int num_labels) {
    // Compute reachability between states
    vector<int> reach = vector<int>(cts.num_abstract_states * cts.num_abstract_states);

    reachability(cts, reach);

    // Sort transitions on label
    std::sort(cts.concrete_transitions.begin(), cts.concrete_transitions.end(),
              [](Transition a, Transition b){ return a.label < b.label;});

    // Find label mutexes
    std::vector<std::pair<int,int>> label_mutexes = std::vector<std::pair<int,int>>();
    for (int label_a = 0; label_a < num_labels; label_a++) {
        // Find first transition with label_a
        int a_t_index = lower_bound(cts.concrete_transitions.begin(), cts.concrete_transitions.end(), label_a,
                             transition_label_comparison) - cts.concrete_transitions.begin();

        for (int label_b = label_a + 1; label_b < num_labels; label_b++) {
            // To check if these labels are operator mutexes, we look through all of their transitions, iff
            // all of them are transition mutexes, they are label mutexes.
            auto at = cts.concrete_transitions.begin() + a_t_index;
            while (at->label == label_a) {
                auto bt = lower_bound(cts.concrete_transitions.begin(), cts.concrete_transitions.end(), label_b,
                                      transition_label_comparison);
                while (bt->label == label_b) {
                    // Lookup their corresponding abstract transitions
                    Transition a_at = cts.lookup_concrete(at);
                    Transition a_bt = cts.lookup_concrete(bt);

                    // If target and src states CAN reach each other, these labels ARE NOT label mutexes
                    if (reach[REACH_XY(a_at.target, a_bt.src)] || reach[REACH_XY(a_bt.target, a_at.src)]) {
                        goto not_label_mutex;
                    }
                    bt++;
                }
                at++;
            }
            label_mutexes.emplace_back(std::pair<int,int>(label_a, label_b));

            not_label_mutex:
            ;
        }

    }
    return label_mutexes;
}

void OpMutexPruningMethod::state_reachability(int int_state, int src_state, CondensedTransitionSystem &cts, std::vector<bool> &reach) {
    for (int state_j = 0; state_j < cts.num_abstract_states; state_j++) {
        if (!reach[REACH_XY(src_state, state_j)]) {
            size_t i;
            for (i = 0; cts.abstract_transitions[i].src != int_state && i < cts.abstract_transitions.size(); ++i);
            for (; cts.abstract_transitions[i].src == int_state && i < cts.abstract_transitions.size(); ++i) {
                if (cts.abstract_transitions[i].target == state_j) {
                    reach[REACH_XY(src_state, state_j)] = true;
                    state_reachability(state_j, src_state, cts, reach);
                    break;
                }
            }
        }
    }
}

void OpMutexPruningMethod::reachability(const CondensedTransitionSystem &cts, std::vector<int> &reach) {
    vector<int> path = vector<int>();
    vector<bool> visited = vector<bool>(cts.num_abstract_states);

    // current states keeps track of the current path
    auto first = cts.abstract_transitions[0];
    path.push_back(first.src);

    // call reach_neighbors with first transitions source as starting state
    reach_neighbors(cts, reach, first.src);
}

void OpMutexPruningMethod::reach_neighbors(const CondensedTransitionSystem &cts, std::vector<int> &reach, const int state) {
    // recursively call reach_neighbors on all neighbors, if it has not previously been visited
    if (!reach[REACH_XY(state, state)]) {
        vector<Transition> neighbors = get_transitions(state, cts);

        reach[REACH_XY(state, state)] = 1;
        for (auto t : neighbors) {
            reach_neighbors(cts, reach, t.target);
            for (int i = 0; i < cts.num_abstract_states; ++i) {
                reach[REACH_XY(state, i)] += reach[REACH_XY(t.target, i)];
            }
        }
    }
}

void OpMutexPruningMethod::reach_print(const CondensedTransitionSystem &cts, const vector<int> &reach) {
    utils::g_log << " ";
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << " " << i;
    }
    utils::g_log << endl;
    for (int i = 0; i < cts.num_abstract_states; ++i) {
        utils::g_log << i;
        for (int j = 0; j < cts.num_abstract_states; ++j) {
            utils::g_log << " " << reach[REACH_XY(i,j)];
        }
        utils::g_log << endl;
    }
}

vector<Transition> get_transitions(const int src, const CondensedTransitionSystem &cts) {
    // TODO: Implement binary search
    vector<Transition> res = vector<Transition>();

    for (auto t : cts.abstract_transitions)
        if (t.src == src)
            res.push_back(t);

    return res;
}

static shared_ptr<OpMutexPruningMethod> _parse(OptionParser &parser) {
    parser.document_synopsis(
            "Operator Mutex Pruning",
            "Operator Mutex Pruning");

    options::Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }

    return make_shared<OpMutexPruningMethod>(opts);
}

static Plugin<OpMutexPruningMethod> _plugin("op_mutex", _parse);

static options::PluginTypePlugin<OpMutexPruningMethod> _type_plugin(
        "op_mutex",
        "Operator mutex type plugin.");
}

