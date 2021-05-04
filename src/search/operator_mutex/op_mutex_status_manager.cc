//
// Created by lasse on 4/29/21.
//
#include "../utils/logging.h"
#include "op_mutex_status_manager.h"
#include "../algorithms/dynamic_bitset.h"

using namespace std;
using namespace dynamic_bitset;

namespace op_mutex {

OpMutexStatusManager::OpMutexStatusManager(int _num_ops, unordered_set<OpMutex> _label_mutexes) : num_ops(_num_ops), label_mutexes(_label_mutexes){
    utils::g_log << "test2" << endl;
}

void OpMutexStatusManager::is_applicable(const State &parent_state, OperatorID op_id, const State &state) {

}

#define ID get_id().get_value()

void OpMutexStatusManager::update_map(const State &parent_state, OperatorID op_id, const State &state) {
//    DynamicBitset<> a = DynamicBitset<>(50);
//    DynamicBitset<> b(a);
//    a.set(4);
//    b.reset();
//    a &= b;
    DynamicBitset<> path(ops_per_state.find(parent_state.ID)->second);
    path.set(op_id.get_index());

    auto st = ops_per_state.find(state.ID);
    if(st != ops_per_state.end()){
        st->second &= path;
    }
    else{
        ops_per_state.insert({state.ID, path});
    }

}
}
