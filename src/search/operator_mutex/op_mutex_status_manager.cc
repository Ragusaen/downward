//
// Created by lasse on 4/29/21.
//
#include "../utils/logging.h"
#include "op_mutex_status_manager.h"
#include "../algorithms/dynamic_bitset.h"

using namespace std;
using namespace dynamic_bitset;

namespace op_mutex {

OpMutexStatusManager::OpMutexStatusManager(int _num_ops) : num_ops(_num_ops){
//    unordered_map<int, DynamicBitset<bool>> abc({{27, DynamicBitset<bool>(50)}, {27, DynamicBitset<bool>(44)}});
//    DynamicBitset<bool> x = DynamicBitset<bool>(50);
    utils::g_log << "test2" << endl;

}

void OpMutexStatusManager::update_operators(const State &parent_state, OperatorID op_id, const State &state) {
    //fix
//    if(ops_per_state.count(state.get_id().get_value())){
//
//    }
//    else{
//    }
}

void OpMutexStatusManager::add_initial(const State &state) {
    ops_per_state.insert({state.get_id().get_value(), DynamicBitset<bool>(num_ops)});
}
}