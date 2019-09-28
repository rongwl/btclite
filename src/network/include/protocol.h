#ifndef BTCLITE_PROTOCOL_H
#define BTCLITE_PROTOCOL_H


#include <array>
#include <cstdint>


/* Services flags */
enum ServiceFlags : uint64_t {
    // Nothing
    kNodeNone = 0,
    
    // kNodeNetwork means that the node is capable of serving the complete block chain. It is currently
    // set by all Bitcoin Core non pruned nodes, and is unset by SPV clients or other light clients.
    kNodeNetwork = (1 << 0),
    
    // kNodeGetutxo means the node is capable of responding to the getutxo protocol request.
    // Bitcoin Core does not support this but a patch set called Bitcoin XT does.
    // See BIP 64 for details on how this is implemented.
    kNodeGetutxo = (1 << 1),
    
    // kNodeBloom means the node is capable and willing to handle bloom-filtered connections.
    // Bitcoin Core nodes used to support this by default, without advertising this bit,
    // but no longer do as of protocol version 70011 (= NO_BLOOM_VERSION)
    kNodeBloom = (1 << 2),
    
    // kNodeWitness indicates that a node can be asked for blocks and transactions including
    // witness data.
    kNodeWitness = (1 << 3),
    
    // kNodeXthin means the node supports Xtreme Thinblocks
    // If this is turned off then the node will not service nor make xthin requests
    kNodeXthin = (1 << 4),
    
    // kNodeNetworkLimited means the same as kNodeNetwork with the limitation of only
    // serving the last 288 (2 day) blocks
    // See BIP159 for details on how this is implemented.
    kNodeNetworkLimited = (1 << 10),

    // Bits 24-31 are reserved for temporary experiments. Just pick a bit that
    // isn't getting used, or one not being used much, and notify the
    // bitcoin-development mailing list. Remember that service bits are just
    // unauthenticated advertisements, so your code must be robust against
    // collisions and other cases where nodes may be advertising a service they
    // do not actually support. Other service bits should be allocated via the
    // BIP process.
};

constexpr ServiceFlags desirable_service_flags = ServiceFlags(kNodeNetwork | kNodeWitness);

/**
 * A shortcut for (services & desirable_service_flags)
 * == desirable_service_flags, ie determines whether the given
 * set of service flags are sufficient for a peer to be "relevant".
 */
static inline bool HasAllDesirableServiceFlags(ServiceFlags services) {
    return !(desirable_service_flags & (~services));
}

static inline bool HasAllDesirableServiceFlags(uint64_t services) {
    return !(desirable_service_flags & (~services));
}


#endif // BTCLITE_PROTOCOL_H
