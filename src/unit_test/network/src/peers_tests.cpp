#include <gtest/gtest.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "peers.h"
#include "constants.h"
#include "node.h"


namespace btclite {
namespace unit_test {

using namespace network;

TEST(PeersTest, Constructor)
{
    Peers peers;
    uint64_t key[4];
    
    ASSERT_EQ(peers.proto_peers().key().size(), 4);
    std::memset(key, 0, sizeof(key));
    EXPECT_TRUE(std::memcmp(peers.key().data(), key, sizeof(key)));
}

TEST(PeersTest, MakeMapKey)
{
    Peers peers;
    NetAddr addr1, addr2;
    
    addr1.SetIpv4(inet_addr("1.2.3.4"));
    addr2.SetIpv4(inet_addr("1.2.5.6"));
    uint64_t key1 = peers.MakeMapKey(addr1, true);
    uint64_t key2 = peers.MakeMapKey(addr1, true);
    EXPECT_EQ(key1, key2);
    key2 = peers.MakeMapKey(addr2, true);
    EXPECT_EQ(key1, key2);
    
    addr2.SetIpv4(inet_addr("1.3.5.6"));
    key2 = peers.MakeMapKey(addr2, true);
    EXPECT_NE(key1, key2);
    
    key1 = peers.MakeMapKey(addr1);
    key2 = peers.MakeMapKey(addr2);
    EXPECT_NE(key1, key2);
    key2 = peers.MakeMapKey(addr1);
    EXPECT_EQ(key1, key2);
}

TEST(PeersTest, AddPeer)
{
    Peers peers;
    NetAddr addr, source;
    bool is_new, is_tried;
    proto_peers::Peer peer;
    std::vector<uint64_t> rand_order_keys;
    uint64_t now = util::GetAdjustedTime();
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    addr.set_timestamp(now-4000);
    source.SetIpv4(inet_addr("1.1.1.1"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_TRUE(is_new);
    EXPECT_FALSE(is_tried);
    EXPECT_EQ(peer.addr(), addr);
    EXPECT_EQ(peer.addr().timestamp(), now-4000);
    uint64_t key = peers.MakeMapKey(addr);
    EXPECT_EQ(peers.rand_order_keys()[0], key);
    
    // update time
    addr.set_timestamp(now);
    ASSERT_FALSE(peers.Add(addr, source, 100));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr().timestamp(), now-100); 
    EXPECT_EQ(peer.addr(), addr);
    
    // delete exist terrible peer
    addr.SetIpv4(inet_addr("1.3.1.1"));
    addr.set_timestamp(now+601);
    ASSERT_TRUE(peers.Add(addr, source));
    key = peers.MakeMapKey(addr);
    EXPECT_EQ(peers.rand_order_keys()[1], key);
    addr.set_timestamp(now);
    ASSERT_FALSE(peers.Add(addr, source));
    EXPECT_FALSE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peers.rand_order_keys().size(), 1);    
    
    // overwrite terrible peer of the same group
    addr.SetIpv4(inet_addr("1.4.1.1"));
    addr.set_timestamp(now+601);
    ASSERT_TRUE(peers.Add(addr, source));
    addr.SetIpv4(inet_addr("1.4.1.2"));
    addr.set_timestamp(now);
    ASSERT_TRUE(peers.Add(addr, source));
    EXPECT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    key = peers.MakeMapKey(addr);
    EXPECT_EQ(peers.rand_order_keys()[1], key);
    EXPECT_EQ(peers.rand_order_keys().size(), 2);
    addr.SetIpv4(inet_addr("1.4.1.1"));
    EXPECT_FALSE(peers.Find(addr, &peer, &is_new, &is_tried)); 
    
    // collision in new_tbl or tried_tbl
    addr.SetIpv4(inet_addr("1.4.1.3"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_FALSE(is_new);
    EXPECT_FALSE(is_tried);
    addr.SetIpv4(inet_addr("1.4.1.2"));
    ASSERT_TRUE(peers.MakeTried(addr));
    addr.SetIpv4(inet_addr("1.4.1.4"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    

    // add mutiple addrs
    std::vector<NetAddr> vec;
    vec.push_back(NetAddr());
    vec[0].SetIpv4(inet_addr("1.1.1.1"));
    vec.push_back(NetAddr());
    vec[1].SetIpv4(inet_addr("2.2.2.2"));
    ASSERT_TRUE(peers.Add(vec, source));
    ASSERT_TRUE(peers.Find(vec[0], &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr(), vec[0]);
    EXPECT_TRUE(is_new);
    EXPECT_FALSE(is_tried);
    EXPECT_TRUE(peers.Find(vec[1], &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr(), vec[1]);
    EXPECT_TRUE(is_new);
    EXPECT_FALSE(is_tried);
    
    // Addr with same IP but diff port does not replace existing addr.
    addr.SetIpv4(inet_addr("3.3.3.3"));
    addr.set_port(8333);
    ASSERT_TRUE(peers.Add(addr, source));
    addr.set_port(8334);
    ASSERT_FALSE(peers.Add(addr, source));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr().port(), 8333);
}

TEST(PeersTest, SetServices)
{
    Peers peers;
    NetAddr addr, source;
    bool is_new, is_tried;
    proto_peers::Peer peer;
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    addr.set_port(8333);
    source.SetIpv4(inet_addr("1.1.1.1"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.SetServices(addr, kNodeNetwork));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr(), addr);
    EXPECT_EQ(peer.addr().services(), kNodeNetwork);
}

TEST(PeersTest, MakePeerTried)
{
    Peers peers;
    NetAddr addr, source;
    bool is_new, is_tried;
    proto_peers::Peer peer;
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    addr.set_timestamp(1000);
    addr.set_port(8333);
    source.SetIpv4(inet_addr("1.1.1.1"));
    ASSERT_TRUE(peers.Add(addr, source));
    
    addr.SetIpv4(inet_addr("1.2.3.5"));
    EXPECT_FALSE(peers.MakeTried(addr));

    addr.SetIpv4(inet_addr("1.2.3.4"));
    addr.set_port(8332);
    EXPECT_FALSE(peers.MakeTried(addr));
    
    addr.SetIpv4(inet_addr("1.1.3.4"));
    EXPECT_FALSE(peers.MakeTried(addr));
    
    addr.SetIpv4(inet_addr("1.2.3.4"));
    addr.set_port(8333);
    ASSERT_TRUE(peers.MakeTried(addr, 2000));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    ASSERT_FALSE(is_new);
    ASSERT_TRUE(is_tried);
    EXPECT_EQ(peer.last_success(), 2000);
    EXPECT_EQ(peer.last_try(), 2000);
    EXPECT_EQ(peer.attempts(), 0);
    EXPECT_FALSE(peers.MakeTried(addr));
}

TEST(PeersTest, SelectPeer)
{
    Peers peers;
    NetAddr addr, source;
    
    // Select from new with 1 addr in new.
    source.SetIpv4(inet_addr("1.1.1.1"));
    addr.SetIpv4(inet_addr("2.2.2.2"));
    addr.set_port(8333);
    ASSERT_TRUE(peers.Add(addr, source));
    proto_peers::Peer out;
    ASSERT_TRUE(peers.Select(&out, true));
    EXPECT_EQ(out.addr(), addr);
    
    // move addr to tried, select from new expected nothing returned.
    ASSERT_TRUE(peers.MakeTried(addr));
    ASSERT_FALSE(peers.Select(&out, true));
    ASSERT_TRUE(peers.Select(&out, false));
    EXPECT_EQ(out.addr(), addr);
}

TEST(PeersTest, AttemptPeer)
{
    Peers peers;
    NetAddr addr, source;
    bool is_new, is_tried;
    proto_peers::Peer peer;
    
    source.SetIpv4(inet_addr("1.1.1.1"));
    addr.SetIpv4(inet_addr("2.2.2.2"));
    addr.set_port(8333);
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.Attempt(addr, 1000));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.attempts(), 1);
    EXPECT_EQ(peer.last_try(), 1000);
    
    addr.SetIpv4(inet_addr("2.2.2.3"));
    EXPECT_FALSE(peers.Attempt(addr, 2000));
    addr.SetIpv4(inet_addr("2.2.2.2"));
    addr.set_port(8334);
    EXPECT_FALSE(peers.Attempt(addr, 2000));
}

TEST(PeersTest, UpdatePeerTime)
{
    Peers peers;
    NetAddr addr, source;
    bool is_new, is_tried;
    proto_peers::Peer peer;
    
    source.SetIpv4(inet_addr("1.1.1.1"));
    addr.SetIpv4(inet_addr("2.2.2.2"));
    addr.set_port(8333);
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.UpdateTime(addr, 20*60+1));
    ASSERT_TRUE(peers.Find(addr, &peer, &is_new, &is_tried));
    EXPECT_EQ(peer.addr().timestamp(), 20*60+1);
    
    addr.SetIpv4(inet_addr("2.2.2.3"));
    EXPECT_FALSE(peers.UpdateTime(addr, 3000));
    addr.SetIpv4(inet_addr("2.2.2.2"));
    addr.set_port(8334);
    EXPECT_FALSE(peers.UpdateTime(addr, 3000));
}

TEST(PeersTest, GetAddresses)
{
    Peers peers;
    NetAddr addr, source;
    int64_t now = util::GetAdjustedTime();
    std::vector<NetAddr> addrs, addrs2;
    
    ASSERT_TRUE(peers.GetAddrs(&addrs));
    EXPECT_TRUE(addrs.empty());
    
    source.SetIpv4(inet_addr("250.1.2.1"));
    addr.SetIpv4(inet_addr("250.250.2.1"));
    addr.set_timestamp(now);
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.MakeTried(addr));
    addr.SetIpv4(inet_addr("250.251.2.2"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.MakeTried(addr));
    addr.SetIpv4(inet_addr("251.252.2.3"));
    ASSERT_TRUE(peers.Add(addr, source));
    addr.SetIpv4(inet_addr("252.253.3.4"));
    ASSERT_TRUE(peers.Add(addr, source));
    addr.SetIpv4(inet_addr("252.254.4.5"));
    ASSERT_TRUE(peers.Add(addr, source));
    
    // GetAddr returns 23% of addresses, 23% of 5 is 1 rounded down.
    ASSERT_TRUE(peers.GetAddrs(&addrs));
    EXPECT_EQ(addrs.size(), 1);
    
    // Ensure GetAddrs still returns 23% when addrman has many addrs.
    for (unsigned int i = 1; i < (8 * 256); i++) {
        int octet1 = i % 256;
        int octet2 = i >> 8 % 256;
        std::string str_addr = std::to_string(octet1) + "." + std::to_string(octet2) + ".1.23";
        NetAddr addr;
        addr.SetIpv4(inet_addr(str_addr.c_str()));
        addr.set_timestamp(now);
        peers.Add(addr, source);
        if (i % 8 == 0)
            peers.MakeTried(addr);
    }
    
    addrs.clear();
    ASSERT_TRUE(peers.GetAddrs(&addrs));
    EXPECT_EQ(addrs.size(), peers.proto_peers().map_peers().size()*kMaxGetaddrPct/100);
    ASSERT_TRUE(peers.GetAddrs(&addrs2));
    EXPECT_EQ(addrs2.size(), peers.proto_peers().map_peers().size()*kMaxGetaddrPct/100);
    EXPECT_NE(addrs[0], addrs2[0]); // Whether GetAddrs returns randomized vector
}

TEST(PeersTest, PeerIsTerrible)
{
    proto_peers::Peer peer;
    int64_t now = 10000000;
    
    peer.set_last_try(now-60);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
    
    peer.set_last_try(now-100);
    peer.mutable_addr()->set_timestamp(now+10*60+1);
    EXPECT_TRUE(peer::IsTerriblePeer(peer, now));
    peer.mutable_addr()->set_timestamp(now+10*60);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
    
    peer.mutable_addr()->set_timestamp(now-kPeerHorizonDays*24*60*60-1);
    EXPECT_TRUE(peer::IsTerriblePeer(peer, now));
    peer.mutable_addr()->set_timestamp(now-kPeerHorizonDays*24*60*60);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
    
    peer.set_attempts(kPeerRetries);
    EXPECT_TRUE(peer::IsTerriblePeer(peer, now));
    peer.set_attempts(kPeerRetries-1);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
    
    peer.set_last_success(now-kMinPeerFailDays*24*60*60-1);
    peer.set_attempts(kMaxPeerFailures);
    EXPECT_TRUE(peer::IsTerriblePeer(peer, now));
    peer.set_last_success(now-kMinPeerFailDays*24*60*60);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
    peer.set_last_success(now-kMinPeerFailDays*24*60*60-1);
    peer.set_attempts(kMaxPeerFailures-1);
    EXPECT_FALSE(peer::IsTerriblePeer(peer, now));
}

TEST(PeersDbTest, Constructor)
{
    PeersDb peers_db(fs::path("/foo"));
    EXPECT_EQ(peers_db.path_peers(), fs::path("/foo") / "peers.dat");
}

TEST(PeersDbTest, DumpAndLoadPeers)
{
    Peers peers;
    NetAddr addr, source;
    int64_t now = util::GetAdjustedTime();
    
    source.SetIpv4(inet_addr("250.1.2.1"));
    addr.SetIpv4(inet_addr("250.250.2.1"));
    addr.set_timestamp(now);
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.MakeTried(addr));
    addr.SetIpv4(inet_addr("250.251.2.2"));
    ASSERT_TRUE(peers.Add(addr, source));
    ASSERT_TRUE(peers.MakeTried(addr));
    addr.SetIpv4(inet_addr("251.252.2.3"));
    ASSERT_TRUE(peers.Add(addr, source));
    addr.SetIpv4(inet_addr("252.253.3.4"));
    ASSERT_TRUE(peers.Add(addr, source));
    addr.SetIpv4(inet_addr("252.254.4.5"));
    ASSERT_TRUE(peers.Add(addr, source));
    
    PeersDb peers_db(fs::path("/tmp"));
    ASSERT_TRUE(peers_db.DumpPeers(peers));
    peers.Clear();
    ASSERT_TRUE(peers_db.LoadPeers(&peers));
    EXPECT_EQ(peers.Size(), 5);
    EXPECT_EQ(peers.proto_peers().new_tbl().size(), 3);
    EXPECT_EQ(peers.proto_peers().tried_tbl().size(), 2);
    
    fs::remove(peers_db.path_peers());
}

} // namespace unit_test
} // namespace btclite
