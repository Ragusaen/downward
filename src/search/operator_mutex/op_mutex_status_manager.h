//
// Created by lasse on 4/29/21.
//

#ifndef FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H
#define FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H

#include "../algorithms/dynamic_bitset.h"
#include "../task_proxy.h"
#include "op_mutex.h"


using namespace dynamic_bitset;
using namespace std;

namespace op_mutex {
class OpMutexStatusManager {
    unordered_map<int, DynamicBitset<>> ops_per_state;
    int num_ops;
    unordered_set<OpMutex> label_mutexes;

public:
    explicit OpMutexStatusManager(int _num_ops, unordered_set<OpMutex> _label_mutexes);
    OpMutexStatusManager() = default;

    void add_initial_state(const State &state);
    void update_state(const State &parent_state, OperatorID op_id, const State &state);
    bool is_applicable(const State &parent_state, OperatorID op_id);
};
}


#endif //FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H
