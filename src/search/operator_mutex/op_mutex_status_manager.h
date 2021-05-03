//
// Created by lasse on 4/29/21.
//

#ifndef FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H
#define FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H

#include "../per_state_bitset.h"
#include "../algorithms/dynamic_bitset.h"
#include "../task_proxy.h"

using namespace dynamic_bitset;
using namespace std;

namespace op_mutex {
class OpMutexStatusManager {
    unordered_map<int, DynamicBitset<bool>> ops_per_state;
    const int num_ops;

public:
    explicit OpMutexStatusManager(int num_operators);

    void add_initial(const State &state);
    void update_operators(const State &parent_state, OperatorID op_id, const State &state);
};
}


#endif //FAST_DOWNWARD_OP_MUTEX_STATUS_MANAGER_H
