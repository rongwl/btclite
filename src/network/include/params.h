#ifndef BTCLITE_NETWORK_PARAMS_H
#define BTCLITE_NETWORK_PARAMS_H


#include "util.h"


namespace btclite {
namespace network {

struct Seed {
    std::string host;
    uint16_t port;
    
    bool operator==(const Seed& b) const;
    bool operator!=(const Seed& b) const;
};

class Params {
public:
    explicit Params(const util::Configuration& config);
    
    Params(BtcNet btcnet, const util::Args& args, 
           const fs::path& path_data_dir);
    
    //-------------------------------------------------------------------------
    uint32_t msg_magic() const;
    
    uint16_t default_port() const;
    
    const std::vector<Seed>& seeds() const;
    
    bool advertise_local_addr() const;
    
    bool discover_local_addr() const;
    
    bool use_dnsseed() const;
    
    const std::vector<std::string>& specified_outgoing() const;
    
    const fs::path& path_data_dir() const;
    
private:
    uint32_t msg_magic_ = 0; // little endian
    uint16_t default_port_ = 0;
    std::vector<Seed> seeds_;
    
    bool advertise_local_addr_ = true;
    bool discover_local_addr_ = true;
    bool use_dnsseed_ = true;
    std::vector<std::string> specified_outgoing_;
    
    fs::path path_data_dir_;
};

} // namespace btclite
} // namespace network

#endif // BTCLITE_NETWORK_PARAMS_H
