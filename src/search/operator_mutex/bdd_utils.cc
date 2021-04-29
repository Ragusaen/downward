#include "bdd_utils.h"
#include "../utils/logging.h"

namespace op_mutex {

void exceptionError(const std::string msg) {
    throw BDDError();
}

std::shared_ptr<Cudd> init_bdd_manager(int num_vars) {
    std::shared_ptr<Cudd> manager = std::make_shared<Cudd>(num_vars);

    manager->setHandler(exceptionError);
    manager->setTimeoutHandler(exceptionError);
    manager->setNodesExceededHandler(exceptionError);

    return manager;
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