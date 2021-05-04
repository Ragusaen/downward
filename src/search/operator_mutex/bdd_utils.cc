#include "bdd_utils.h"
#include "../utils/logging.h"

namespace op_mutex {

void exceptionError(const std::string) {
    throw BDDError();
}

void init_bdd_manager(Cudd &bdd_manager) {
    bdd_manager.setHandler(exceptionError);
    bdd_manager.setTimeoutHandler(exceptionError);
    bdd_manager.setNodesExceededHandler(exceptionError);

    bdd_manager.AutodynEnable(CUDD_REORDER_SYMM_SIFT);
    bdd_manager.ReduceHeap(CUDD_REORDER_SYMM_SIFT, 3000);
}


BDD mergeAndBDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.And(bdd2, maxSize);
}
BDD mergeOrBDD(const BDD &bdd, const BDD &bdd2, int maxSize) {
    return bdd.Or(bdd2, maxSize);
}
ADD mergeSumADD(const ADD &add, const ADD &add2, int ) {
    return add + add2;
}

ADD mergeMaxADD(const ADD &add, const ADD &add2, int ) {
    return add.Maximum(add2);
}
}