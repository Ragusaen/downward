#include "op_mutex.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include "../merge_and_shrink/merge_and_shrink_algorithm.h"
#include "../merge_and_shrink/transition_system.h"
#include "../merge_and_shrink/label_equivalence_relation.h"
#include "../merge_and_shrink/labels.h"
#include "CondensedTransitionSystem.h"
#include <cassert>
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



FactoredTransitionSystem* OpMutexPruningMethod::run(FactoredTransitionSystem* fts) {
    utils::g_log << "Operator mutex running" << endl;

    TransitionSystem ts = fts->get_transition_system(fts->get_size() - 1);
    std::vector<Transition> transitions = flatten(ts.get_transitions());

    for (Transition t: transitions) {
        for (Transition s: transitions) {
            if (t.src == s.target && t.target == s.src)
                goto found;
        }
        utils::g_log << "(" << t.src << "," << t.target << ")" << endl;
        found:
        ;
    }

    std::vector<Transition> k_ts = std::vector<Transition>();
    k_ts.push_back(Transition(0, 1));
    k_ts.push_back(Transition(1, 0));
    k_ts.push_back(Transition(0, 3));
    k_ts.push_back(Transition(2, 1));
    k_ts.push_back(Transition(3, 5));
    k_ts.push_back(Transition(2, 5));
    k_ts.push_back(Transition(5, 4));
    k_ts.push_back(Transition(4, 3));

    CondensedTransitionSystem cts = CondensedTransitionSystem(k_ts, 6);

    utils::g_log << "Concrete to abstract conversion" << std::endl;
    for (int i = 0; i < cts.concrete_to_abstract_state.size(); i++) {
        utils::g_log << i << " -> " << cts.concrete_to_abstract_state[i] << std::endl;
    }

    utils::g_log << "Abstract transitions" << std::endl;
    for (Transition at : cts.abstract_transitions) {
        utils::g_log << to_string(at) << std::endl;
    }

    auto ts_mutexes = infer_transition_mutex_in_condensed_ts(cts);

    for (auto mutex : ts_mutexes) {
        utils::g_log << to_string(mutex.first) << " is transition mutex with " << to_string(mutex.second) << std::endl;
    }

    utils::g_log << "Operator mutex done" << endl;
    return fts;
}

#define REACH_XY(x, y) (x * cts.num_abstract_states + y)
std::vector<std::pair<Transition, Transition>>
OpMutexPruningMethod::infer_transition_mutex_in_condensed_ts(CondensedTransitionSystem cts) {
    auto transition_mutexes = std::vector<std::pair<Transition, Transition>>();

    // Add transition mutexes for each pair of transitions that share start and end state
    for (size_t i = 0; i < cts.abstract_transitions.size(); i++) {
        auto at1 = cts.abstract_transitions[i];
        for (size_t j = i + 1; j < cts.abstract_transitions.size(); j++) {
            auto at2 = cts.abstract_transitions[j];
            if (at1.src == at2.src && at1.target == at2.target) {
                transition_mutexes.push_back(std::pair<Transition,Transition>(at1, at2));
            }
        }
    }

    //// Compute transition mutexes between transitions where the start and end states are not reachable between each other
    // Compute reachability between states
    std::vector<bool> reach = std::vector<bool>(cts.num_abstract_states * cts.num_abstract_states);

    for (int state = 0; state < cts.num_abstract_states; state++) {
        reach[REACH_XY(state, state)] = true;
        state_reachability(state, state, cts, reach);
    }

    // Find transition mutexes
    for (int state_i = 0; state_i < cts.num_abstract_states; state_i++) {
        for (int state_j = 0; state_j < cts.num_abstract_states; state_j++) {
            if (!reach[REACH_XY(state_i, state_j)]) {

                for (size_t k = 0; k < cts.abstract_transitions.size(); k++) {
                    for (size_t l = k + 1; l < cts.abstract_transitions.size(); l++) {
                        if (k == l)
                            continue;
                        auto at1 = cts.abstract_transitions[k];
                        auto at2 = cts.abstract_transitions[l];
                        if (at1.target == state_i && at2.src == state_j) {

                            if (!reach[REACH_XY(at2.target, at1.src)]) {
                                transition_mutexes.push_back(std::pair<Transition, Transition>(at1, at2));
                            }
                        }
                    }
                }
            }
        }
    }

    return transition_mutexes;
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

