#include "bdd_utils.h"
#include "../utils/logging.h"

namespace op_mutex {

void exceptionError(const std::string) {
    throw BDDError();
}

void init_bdd_manager(Cudd &bdd_manager, int variable_ordering_method) {
    bdd_manager.setHandler(exceptionError);
    bdd_manager.setTimeoutHandler(exceptionError);
    bdd_manager.setNodesExceededHandler(exceptionError);

    if (variable_ordering_method == 1) {
        bdd_manager.AutodynEnable(CUDD_REORDER_ANNEALING);
    } else if (variable_ordering_method == 2) {
        bdd_manager.AutodynEnable(CUDD_REORDER_GENETIC);
    } else if (variable_ordering_method == 3) {
        bdd_manager.AutodynEnable(CUDD_REORDER_RANDOM);
    } else if (variable_ordering_method == 4) {
        bdd_manager.AutodynEnable(CUDD_REORDER_WINDOW2);
    } else if (variable_ordering_method == 5) {
        bdd_manager.AutodynEnable(CUDD_REORDER_SYMM_SIFT);
    }

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