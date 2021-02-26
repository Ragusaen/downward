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


    utils::g_log << "Operator mutex done" << endl;
    return fts;
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

