#ifndef BTCLITE_ENVIRONMENT_PARAMS_H
#define BTCLITE_ENVIRONMENT_PARAMS_H

#include <cstdint>

// Base environment for params initialization in other module.
enum class BaseEnv : uint8_t
{
    none,
    mainnet,
    testnet,
    regtest
};

#endif // BTCLITE_ENVIRONMENT_PARAMS_H
