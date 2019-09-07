#include "net.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "config.h"
#include "constants.h"
#include "network/include/params.h"
#include "message_types/messages.h"


bool LocalNetConfig::IsLocal(const btclite::NetAddr& addr)
{
    LOCK(cs_local_net_config_);
    for (auto it = local_addrs_.begin(); it != local_addrs_.end(); ++it)
        if (*it == addr)
            return true;
    
    return false;
}

bool LocalNetConfig::LookupLocalAddrs()
{
    // Get local host ip
    struct ifaddrs* myaddrs;
    
    if (getifaddrs(&myaddrs))
        return false;
    
    for (struct ifaddrs* ifa = myaddrs; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;
        if ((ifa->ifa_flags & IFF_UP) == 0)
            continue;
        if (strcmp(ifa->ifa_name, "lo") == 0)
            continue;
        if (strcmp(ifa->ifa_name, "lo0") == 0)
            continue;
        
        if (ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in* s4 = reinterpret_cast<struct sockaddr_in*>(ifa->ifa_addr);
            btclite::NetAddr addr;
            addr.SetIpv4(s4->sin_addr.s_addr);
            if (AddLocalHost(addr))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv4 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
        else if (ifa->ifa_addr->sa_family == AF_INET6)
        {
            struct sockaddr_in6* s6 = reinterpret_cast<struct sockaddr_in6*>(ifa->ifa_addr);
            btclite::NetAddr addr;
            addr.SetIpv6(s6->sin6_addr.s6_addr);
            if (AddLocalHost(addr))
                BTCLOG(LOG_LEVEL_INFO) << "Add local IPv6 addr " << ifa->ifa_name << ":" << addr.ToString();
        }
    }
    freeifaddrs(myaddrs);
    
    return !local_addrs_.empty();  
}

bool LocalNetConfig::AddLocalHost(const btclite::NetAddr& addr)
{
    if (!addr.IsRoutable())
        return false;

    LOCK(cs_local_net_config_);
    if (find(local_addrs_.begin(), local_addrs_.end(), addr) == local_addrs_.end())
    {
        local_addrs_.push_back(addr);
    }

    return true;
}

bool MessageHeader::IsValid() const
{
    if (magic_ != Network::SingletonParams::GetInstance().msg_magic()) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ << ") is invalid";
        return false;
    }
    
    if (command_ != btc_message::Version::command &&
            command_ != "verack" &&
            command_ != "addr" &&
            command_ != "inv" &&
            command_ != "getdata" &&
            command_ != "merkleblock" &&
            command_ != "getblocks" &&
            command_ != "getheaders" &&
            command_ != "tx" &&
            command_ != "headers" &&
            command_ != "block" &&
            command_ != "getaddr" &&
            command_ != "mempool" &&
            command_ != "ping" &&
            command_ != "pong" &&
            command_ != "notfound" &&
            command_ != "filterload" &&
            command_ != "filteradd" &&
            command_ != "filterclear" &&
            command_ != "reject" &&
            command_ != "sendheaders" &&
            command_ != "feefilter" &&
            command_ != "sendcmpct" &&
            command_ != "cmpctblock" &&
            command_ != "getblocktxn" &&
            command_ != "blocktxn") {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::command_(" << command_ << ") is invalid";
        return false;
    }
    
    if (payload_length_ > max_message_size) {
        BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::payload_length_(" << payload_length_ << ") is invalid";
        return false;
    }
    
    return true;
}

void MessageHeader::ReadRawData(const uint8_t *in)
{
    if (!in)
        return;
    
    magic_ = *reinterpret_cast<const uint32_t*>(in);
    in += sizeof(magic_);
    
    const char *command = reinterpret_cast<const char*>(in);
    command_ = std::move(std::string(command, command + strnlen(command, COMMAND_SIZE)));
    in += COMMAND_SIZE;
    
    payload_length_ = *reinterpret_cast<const uint32_t*>(in);
    in += sizeof(payload_length_);
    
    checksum_ = *reinterpret_cast<const uint32_t*>(in);
}

void MessageHeader::WriteRawData(uint8_t *cout)
{

}

void Message::DataFactory(const uint8_t *data_raw)
{
    if (header_.command() == btc_message::Version::command)
        data_ = std::make_shared<btc_message::Version>(data_raw);
    else
        data_.reset();
}

NetArgs::NetArgs(const Args& args)
    : is_listen_(args.GetBoolArg(FULLNODE_OPTION_LISTEN, true)),
      is_discover_(args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true)),
      is_dnsseed_(args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true)),
      specified_outgoing_()
{
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT))
        specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
}
