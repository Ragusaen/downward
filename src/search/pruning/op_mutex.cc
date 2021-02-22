#include "op_mutex.h"

#include "../option_parser.h"
#include "../plugin.h"

#include "../utils/logging.h"

#include "../merge_and_shrink/merge_and_shrink_algorithm.h"
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
using merge_and_shrink::FactoredTransitionSystem;

namespace op_mutex_pruning {
OpMutexPruningMethod::OpMutexPruningMethod(const Options &opts) :
    size(opts.get<int>("size"))
    {}

FactoredTransitionSystem* OpMutexPruningMethod::run(FactoredTransitionSystem* fts) {
    utils::g_log << "Operator mutex running" << endl;

    return fts;
}

static shared_ptr<OpMutexPruningMethod> _parse(OptionParser &parser) {
    parser.document_synopsis(
            "Operator Mutex Pruning",
            "Operator Mutex Pruning");

    parser.add_option<int>(
            "size",
            "Operator mutex pruning [internal].",
            "20"
            );

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

