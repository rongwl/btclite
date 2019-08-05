#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ifaddrs.h>

#include "config.h"
#include "constants.h"
#include "net.h"

void LocalNetConfig::LookupLocalAddrs()
{
    // Get local host ip
    struct ifaddrs* myaddrs;
    if (getifaddrs(&myaddrs) == 0)
    {
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
    }
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

bool MessageHeader::IsValid(BaseEnv env) const
{
    if ((env == BaseEnv::mainnet && magic_ != main_magic) ||
            (env == BaseEnv::testnet && magic_ != testnet_magic) ||
            (env == BaseEnv::regtest && magic_ != regtest_magic)) {
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

void Message::DataFactory(const std::string& command)
{
    if (command == btc_message::Version::command)
        data_ = std::make_shared<btc_message::Version>();
    else
        data_.reset();
}

bool Message::RecvMsgHandle()
{
    if (!data_.use_count())
        return false;
    
    return data_.get()->RecvMsgHandle();
}

NetArgs::NetArgs()
    : is_listen_(ExecutorConfig::args().GetBoolArg(FULLNODE_OPTION_LISTEN, true)),
      is_discover_(ExecutorConfig::args().GetBoolArg(FULLNODE_OPTION_DISCOVER, true)),
      is_dnsseed_(ExecutorConfig::args().GetBoolArg(FULLNODE_OPTION_DNSSEED, true)),
      specified_outgoing_()
{
    if (ExecutorConfig::args().IsArgSet(FULLNODE_OPTION_CONNECT))
        specified_outgoing_ = ExecutorConfig::args().GetArgs(FULLNODE_OPTION_CONNECT);
}
