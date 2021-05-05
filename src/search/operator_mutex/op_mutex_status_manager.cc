//
// Created by lasse on 4/29/21.
//
#include "../utils/logging.h"
#include "op_mutex_status_manager.h"
#include "../algorithms/dynamic_bitset.h"
#include "op_mutex.h"

#define ID get_id().get_value()

using namespace std;
using namespace dynamic_bitset;

namespace op_mutex {

OpMutexStatusManager::OpMutexStatusManager(int _num_ops, unordered_set<OpMutex> _label_mutexes) : num_ops(_num_ops),
                                                                                                  label_mutexes(
                                                                                                          _label_mutexes) {
    utils::g_log << "test2" << endl;
}

bool OpMutexStatusManager::is_applicable(const State &parent_state, OperatorID op_id) {
    DynamicBitset<> used_ops = ops_per_state.find(parent_state.ID)->second;
    int next_op = op_id.get_index();
    for (int i = 0; i < num_ops; i++) {
        if (used_ops[i]) {
            if (label_mutexes.count(OpMutex(i, next_op))) {
                return false;
            }
        }
    }
    return true;
}

void OpMutexStatusManager::update_state(const State &parent_state, OperatorID op_id, const State &state) {
    DynamicBitset<> path(ops_per_state.find(parent_state.ID)->second);
    path.set(op_id.get_index());

    auto st = ops_per_state.find(state.ID);
    if (st != ops_per_state.end()) {
        st->second |= path;
    } else {
        ops_per_state.insert({state.ID, path});
    }

}

void OpMutexStatusManager::add_initial_state(const State &state) {
    ops_per_state.insert({state.ID, DynamicBitset<>(num_ops)});
}
}
