
#ifndef FAST_DOWNWARD_OP_MUTEX2_H
#define FAST_DOWNWARD_OP_MUTEX2_H

#include <unordered_set>

namespace op_mutex2 {
struct OpMutex {
    int label1;
    int label2;

    OpMutex(int l1, int l2) : label1(l1), label2(l2) {}
};

static inline bool operator==(const OpMutex a, const OpMutex b) {
    return a.label1 == b.label1 && a.label2 == b.label2;
}
}

namespace std {
template<>
struct hash<op_mutex2::OpMutex> {
    std::size_t operator()(const op_mutex2::OpMutex &p) const noexcept {
        return p.label1 ^ p.label2 << 16; // Bitshift 16 bits to create least overlap between labels
    }
};
}

#endif //FAST_DOWNWARD_OP_MUTEX2_H
