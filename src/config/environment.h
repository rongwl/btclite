#ifndef BTCLITE_ENVIRONMENT_PARAMS_H
#define BTCLITE_ENVIRONMENT_PARAMS_H

#include <cstdint>

// For params initialization in other module.
enum class Environment : uint8_t
{
    mainnet,
    testnet,
    regtest
};

#endif // BTCLITE_ENVIRONMENT_PARAMS_H
