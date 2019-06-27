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
    MessageHeader::MsgMagic msg_magic_;
    uint16_t default_port_;
    std::vector<Seed> seeds_;
};

class SingletonParams {
public:
    static Params& GetInstance(BaseEnv env)
    {
        static Params params(env);
        return params;
    }
    
    static Params& GetInstance()
    {
        return GetInstance(BaseEnv::mainnet);
    }
    
    SingletonParams(const SingletonParams&) = delete;
    SingletonParams& operator=(const SingletonParams&) = delete;
    
private:
    SingletonParams() {}        
};

} // namespace Network

#endif // BTCLITE_NETWORK_PARAMS_H
