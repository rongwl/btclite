#ifndef BTCLITE_NETWORK_PARAMS_H
#define BTCLITE_NETWORK_PARAMS_H


#include "util.h"


namespace btclite {
namespace network {

struct Seed {
    std::string host;
    uint16_t port;
    
    bool operator==(const Seed& b) const
    {
        return (host == b.host && port == b.port);
    }
    
    bool operator!=(const Seed& b) const
    {
        return !(*this == b);
    }
};

class Params {
public:
    Params(BaseEnv env, const util::Args& args);
    
    //-------------------------------------------------------------------------
    uint32_t msg_magic() const
    {
        return msg_magic_;
    }
    
    uint16_t default_port() const
    {
        return default_port_;
    }
    
    const std::vector<Seed>& seeds() const
    {
        return seeds_;
    }
    
    bool advertise_local_addr() const
    {
        return advertise_local_addr_;
    }
    
    bool discover_local_addr() const
    {
        return discover_local_addr_;
    }
    
    bool use_dnsseed() const
    {
        return use_dnsseed_;
    }
    
    const std::vector<std::string>& specified_outgoing() const
    {
        return specified_outgoing_;
    }
    
private:
    uint32_t msg_magic_ = 0; // little endian
    uint16_t default_port_ = 0;
    std::vector<Seed> seeds_;
    
    bool advertise_local_addr_ = true;
    bool discover_local_addr_ = true;
    bool use_dnsseed_ = true;
    std::vector<std::string> specified_outgoing_;
};

} // namespace btclite
} // namespace network

#endif // BTCLITE_NETWORK_PARAMS_H
