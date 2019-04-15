#ifndef BTCLITE_RANDOM_H
#define BTCLITE_RANDOM_H

#include <cstdint>

class Random {
public:
    static uint64_t Get(uint64_t max);
};

#endif // BTCLITE_RANDOM_H
