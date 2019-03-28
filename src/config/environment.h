#ifndef BTCLITE_ENVIRONMENT_PARAMS_H
#define BTCLITE_ENVIRONMENT_PARAMS_H

#include <cstdint>

// Network environment for params initialization in other module.
enum class NetworkEnv : uint8_t
{
	none,
    mainnet,
    testnet,
    regtest
};

#endif // BTCLITE_ENVIRONMENT_PARAMS_H
