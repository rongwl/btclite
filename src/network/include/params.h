#ifndef BTCLITE_NETWORK_PARAMS_H
#define BTCLITE_NETWORK_PARAMS_H

#include "environment.h"
#include "net.h"

struct Seed {
    std::string host_;
    uint16_t port_;
    
    bool operator==(const Seed& b) const
    {
        return (host_ == b.host_ && port_ == b.port_);
    }
    
    bool operator!=(const Seed& b) const
    {
        return !(*this == b);
    }
};

namespace Network {

class Params {
public:
    bool Init(NetworkEnv env);
    
    //-------------------------------------------------------------------------
    MessageHeader::MsgMagic msg_magic()
    {
        return msg_magic_;
    }
    
    uint16_t default_port()
    {
        return default_port_;
    }
    
    const std::vector<Seed>& seeds() const
    {
        return seeds_;
    }
    
private:
    MessageHeader::MsgMagic msg_magic_;
    uint16_t default_port_;
    std::vector<Seed> seeds_;
};

} // namespace Network

#endif // BTCLITE_NETWORK_PARAMS_H
