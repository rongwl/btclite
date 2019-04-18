#include <random>

#include "random.h"

uint64_t Random::Get(uint64_t max)
{
    if (max == 0)
        return 0;

    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 rng(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<uint64_t> dis(0, max); 
    
    return dis(rng);
}
