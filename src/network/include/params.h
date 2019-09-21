#ifndef BTCLITE_NETWORK_PARAMS_H
#define BTCLITE_NETWORK_PARAMS_H


#include "environment.h"
#include "net.h"


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

namespace Network {

class Params {
public:
    Params(BaseEnv env);
    
    //-------------------------------------------------------------------------
    MessageHeader::MsgMagic msg_magic() const
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
    
private:
    MessageHeader::MsgMagic msg_magic_; // little endian
    uint16_t default_port_;
    std::vector<Seed> seeds_;
};

class SingletonParams : Uncopyable {
public:
    static Params& GetInstance(BaseEnv env = BaseEnv::mainnet)
    {
        static Params params(env);
        return params;
    }

private:
    SingletonParams() {}        
};

} // namespace Network

#endif // BTCLITE_NETWORK_PARAMS_H
