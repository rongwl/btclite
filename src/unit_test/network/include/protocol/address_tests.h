#include <gtest/gtest.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "protocol/address.h"


using namespace btclite::network::protocol;

class AddrTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        addr_.SetIpv4(inet_addr("1.2.3.1"));
        msg_addr2_.mutable_addr_list()->push_back(addr_);
        addr_.SetIpv4(inet_addr("1.2.3.2"));
        msg_addr2_.mutable_addr_list()->push_back(addr_);
    }
    
    btclite::network::NetAddr addr_;
    Addr msg_addr1_;
    Addr msg_addr2_;
};
