
#ifndef FAST_DOWNWARD_OP_MUTEX_H
#define FAST_DOWNWARD_OP_MUTEX_H

#include <unordered_set>
#include <string>

namespace op_mutex {
struct OpMutex {
    int label1;
    int label2;

    OpMutex(int l1, int l2) : label1(l1), label2(l2) {}
};

static inline bool operator==(const OpMutex a, const OpMutex b) {
    return a.label1 == b.label1 && a.label2 == b.label2;
}


    __attribute__((unused)) static std::string to_string(OpMutex op_mutex) {
    return "(" + std::to_string(op_mutex.label1) + ", " + std::to_string(op_mutex.label2) + ")";
}
}

namespace std {
template<>
struct hash<op_mutex::OpMutex> {
    std::size_t operator()(const op_mutex::OpMutex &p) const noexcept {
        return p.label1 ^ p.label2 << 16; // Bitshift 16 bits to create least overlap between labels
    }
};
}

#endif //FAST_DOWNWARD_OP_MUTEX_H
