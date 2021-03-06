#include "peers.h"

#include "hash.h"
#include "protocol/message.h"


namespace btclite {
namespace network {

namespace peer {

bool IsTerriblePeer(const proto_peers::Peer& peer, int64_t now)
{
    // never remove things tried in the last minute
    if (peer.last_try() && peer.last_try() >= now - 60) 
        return false;

    // came in a flying DeLorean
    if (peer.addr().timestamp() > now + 10 * 60) 
        return true;

    // not seen in recent history
    if (peer.addr().timestamp() == 0 || 
            now - peer.addr().timestamp() > kPeerHorizonDays * 24 * 60 * 60) 
        return true;

    // tried N times and never a success
    if (peer.last_success() == 0 && peer.attempts() >= kPeerRetries) 
        return true;

    // N successive failures in the last week
    if (now - peer.last_success() > kMinPeerFailDays * 24 * 60 * 60 && 
            peer.attempts() >= kMaxPeerFailures) 
        return true;

    return false;
}

} // namespace peer

Peers::Peers()
    : key_(util::RandHash256())
{
    proto_peers_.mutable_key()->Resize(4, 0);
    std::memcpy(proto_peers_.mutable_key()->begin(), key_.begin(), key_.size());
}

bool Peers::Add(const NetAddr &addr, 
                         const NetAddr& source, int64_t time_penalty)
{
    proto_peers::Peer exist_peer;
    bool is_new, is_tried;
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    if (!addr.IsRoutable())
        return false;
    
    // Do not set a penalty for a source's self-announcement
    if (addr == source) {
        time_penalty = 0;
    }
    
    LOCK(cs_peers_);
    // addr exists in map_peers
    if (Find(map_key, group_key, &exist_peer, &is_new, &is_tried)) {
        // addr exists in new_tbl or tried_tbl and the peer is terrible
        if (peer::IsTerriblePeer(exist_peer)) {
            if (is_new)
                proto_peers_.mutable_new_tbl()->erase(group_key);
            else if (is_tried)
                proto_peers_.mutable_tried_tbl()->erase(group_key);
            proto_peers_.mutable_map_peers()->erase(map_key);
            EraseRand(map_key);
            
            return false;
        }
        
        // do not update if no new information is present
        if (!addr.timestamp() || 
            (exist_peer.addr().timestamp() && 
             addr.timestamp() <= exist_peer.addr().timestamp()))
            return false;
        
        // periodically update nTime
        bool currently_online = (util::GetAdjustedTime() - addr.timestamp()
                                 < 24 * 60 * 60);
        uint32_t update_interval = (currently_online ? 60 * 60 : 24 * 60 * 60);
        if (addr.timestamp() &&
            (!exist_peer.addr().timestamp() || 
             exist_peer.addr().timestamp() < addr.timestamp() - update_interval - time_penalty))
            proto_peers_.mutable_map_peers()->find(map_key)->second.mutable_addr()->set_timestamp(
                std::max((int64_t)0, addr.timestamp() - time_penalty));

        // add services
        proto_peers_.mutable_map_peers()->find(map_key)->second.mutable_addr()->set_services(
            ServiceFlags(exist_peer.addr().services() | addr.services()));
        
        return false;
    }
    
    // addr does't exist in map_peers, but there's same group addr in new_tbl or tried_tbl
    uint64_t key;
    bool add_new = true;
    if (FindSameGroup(group_key, &exist_peer, &is_tried, &key)) {
        if (peer::IsTerriblePeer(exist_peer)) {
            if (is_tried)
                proto_peers_.mutable_tried_tbl()->erase(group_key);
            else
                proto_peers_.mutable_new_tbl()->erase(group_key);
            proto_peers_.mutable_map_peers()->erase(key);
            EraseRand(key);
        }
        else
            add_new = false;
    }
    
    // add addr into map_peers and new_tbl
    proto_peers::Peer new_peer;
    new_peer.set_allocated_addr(new proto_netaddr::NetAddr(addr.proto_addr()));
    new_peer.set_allocated_source(new proto_netaddr::NetAddr(source.proto_addr()));
    new_peer.set_attempts(0);
    proto_peers_.mutable_map_peers()->insert(
        google::protobuf::MapPair<uint64_t, proto_peers::Peer>(map_key, new_peer));
    rand_order_keys_.push_back(map_key);
    if (add_new)
        proto_peers_.mutable_new_tbl()->insert(
            google::protobuf::MapPair<uint64_t, uint64_t>(group_key, map_key));
        
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added " << addr.ToString() << " from " << source.ToString() 
                              << ", tried(" << proto_peers_.tried_tbl().size() << ") new("
                              << proto_peers_.new_tbl().size() << ")";
    
    return true;
}

bool Peers::Add(const std::vector<NetAddr> &vec_addr,
                         const NetAddr& source, int64_t time_penalty)
{
    int added = 0;
    
    for (auto it = vec_addr.begin(); it != vec_addr.end(); ++it)
        added += Add(*it, source, time_penalty) ? 1 : 0;
    if (added)
        BTCLOG(LOG_LEVEL_INFO) << "Added " << added << " addrs from " << source.ToString() 
                               << ", tried(" << proto_peers_.tried_tbl().size() << ") new("
                               << proto_peers_.new_tbl().size() << ")";
    
    return added > 0;
}

bool Peers::MakeTried(const NetAddr& addr, int64_t time)
{
    proto_peers::Peer exist_peer;
    bool is_new, is_tried;
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    LOCK(cs_peers_);
    if (!Find(map_key, group_key, &exist_peer, &is_new, &is_tried))
        return false;
    
    // check whether we are talking about the exact same NetAddr (including same port)
    if (NetAddr(exist_peer.addr()) != addr)
        return false;
    
    // update info
    proto_peers_.mutable_map_peers()->find(map_key)->second.set_last_success(time);
    proto_peers_.mutable_map_peers()->find(map_key)->second.set_last_try(time);
    proto_peers_.mutable_map_peers()->find(map_key)->second.set_attempts(0);
    // nTime is not updated here, to avoid leaking information about
    // currently-connected peers.
    
    // if it is already in the tried map, don't do anything else
    if (is_tried)
        return false;
    
    proto_peers_.mutable_tried_tbl()->insert(
        google::protobuf::MapPair<uint64_t, uint64_t>(group_key, map_key));
    proto_peers_.mutable_new_tbl()->erase(group_key);
    
    BTCLOG(LOG_LEVEL_INFO) << "Moving " << addr.ToString() << " to tried.";
    
    return true;
}

bool Peers::Select(proto_peers::Peer *out, bool new_only) const
{
    if (!out)
        return false;
    
    LOCK(cs_peers_);
    
    if (proto_peers_.new_tbl().size() == 0 && proto_peers_.tried_tbl().size() == 0)
        return false;
    
    if (new_only && proto_peers_.new_tbl().size() == 0)
        return false;
    
    // Use a 50% chance for choosing between tried and new peers.
    if (!new_only &&
        proto_peers_.tried_tbl().size() > 0 &&
        (proto_peers_.new_tbl().size() == 0 || util::RandUint64(1) == 0)) {
        // use a tried peer
        uint64_t rand_pos = util::RandUint64(proto_peers_.tried_tbl().size()-1);
        auto it = proto_peers_.tried_tbl().begin();
        while (rand_pos-- > 0 && it++ != proto_peers_.tried_tbl().end())
            ;
       *out = proto_peers_.map_peers().find(it->second)->second;
    }
    else {
        // use a new peer
        uint64_t rand_pos = util::RandUint64(proto_peers_.new_tbl().size()-1);
        auto it = proto_peers_.new_tbl().begin();
        while (rand_pos-- > 0 && it++ != proto_peers_.new_tbl().end())
            ;
        *out = proto_peers_.map_peers().find(it->second)->second;
    }
    
    return true;
}

bool Peers::SetServices(const NetAddr& addr, uint64_t services)
{
    proto_peers::Peer exist_peer;
    bool is_new, is_tried;
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    LOCK(cs_peers_);
    if (!Find(map_key, group_key, &exist_peer, &is_new, &is_tried))
        return false;
    
    if (NetAddr(exist_peer.addr()) != addr)
        return false;
    
    proto_peers_.mutable_map_peers()->find(map_key)->second.mutable_addr()->set_services(services);
    
    return true;
}

bool Peers::Find(uint64_t map_key, uint64_t group_key, proto_peers::Peer *out,
                          bool *is_new, bool *is_tried)
{
    if (!out || !is_new || !is_tried)
        return false;
    
    LOCK(cs_peers_);
    auto it = proto_peers_.mutable_map_peers()->find(map_key);
    if (it != proto_peers_.mutable_map_peers()->end()) {
        auto it2 = proto_peers_.tried_tbl().find(group_key);
        if (it2 != proto_peers_.tried_tbl().end() && it2->second == map_key)
            *is_tried = true;
        else
            *is_tried = false;
        
        it2 = proto_peers_.new_tbl().find(group_key);
        if (it2 != proto_peers_.new_tbl().end() && it2->second == map_key)
            *is_new = true;
        else
            *is_new = false;
        
        *out = it->second;
        
        return true;
    }
    
    return false;
}

bool Peers::Find(const NetAddr& addr, proto_peers::Peer *out,
                          bool *is_new, bool *is_tried)
{
    if (!out || !is_tried)
        return false;
    
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    return Find(map_key, group_key, out, is_new, is_tried);
}

bool Peers::FindSameGroup(uint64_t group_key, proto_peers::Peer *out, bool *is_tried,
                                   uint64_t *key)
{
    if (!out)
        return false;
    
    auto it = proto_peers_.new_tbl().find(group_key);
    if (it != proto_peers_.new_tbl().end()) {
        if (key)
            *key = it->second;
        *out = proto_peers_.mutable_map_peers()->find(it->second)->second;
        *is_tried = false;
        
        return true;
    }
    
    it = proto_peers_.tried_tbl().find(group_key);
    if (it != proto_peers_.tried_tbl().end()) {
        if (key)
            *key = it->second;
        *out = proto_peers_.mutable_map_peers()->find(it->second)->second;
        *is_tried = true;
        
        return true;
    }
    
    return false;
}

bool Peers::Attempt(const NetAddr &addr, int64_t time)
{
    proto_peers::Peer exist_peer;
    bool is_new, is_tried;
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    LOCK(cs_peers_);
    if (!Find(map_key, group_key, &exist_peer, &is_new, &is_tried))
        return false;
    
    // check whether we are talking about the exact same NetAddr (including same port)
    if (NetAddr(exist_peer.addr()) != addr)
        return false;
    
    proto_peers_.mutable_map_peers()->find(map_key)->second.set_last_try(time);
    proto_peers_.mutable_map_peers()->find(map_key)->second.set_attempts(exist_peer.attempts()+1);
    
    return true;
}

bool Peers::UpdateTime(const NetAddr &addr, int64_t time)
{
    proto_peers::Peer exist_peer;
    bool is_new, is_tried;
    uint64_t map_key = MakeMapKey(addr);
    uint64_t group_key = MakeMapKey(addr, true);
    
    LOCK(cs_peers_);
    if (!Find(map_key, group_key, &exist_peer, &is_new, &is_tried))
        return false;
    
    // check whether we are talking about the exact same NetAddr (including same port)
    if (NetAddr(exist_peer.addr()) != addr)
        return false;
    
    if (time - exist_peer.addr().timestamp() > 20*60)
        proto_peers_.mutable_map_peers()->find(map_key)->second.mutable_addr()->set_timestamp(time);
    
    return true;
}

bool Peers::GetAddrs(std::vector<NetAddr> *out)
{
    if (!out)
        return false;
    
    LOCK(cs_peers_);
    
    uint32_t count = kMaxGetaddrPct*proto_peers_.map_peers().size()/100 < kMaxGetaddrCount ? 
                     kMaxGetaddrPct*proto_peers_.map_peers().size()/100 : kMaxGetaddrCount;

    std::random_shuffle(rand_order_keys_.begin(), rand_order_keys_.end());
    for (int i = 0; i < count; i++) {
        const proto_peers::Peer& peer = proto_peers_.map_peers().find(rand_order_keys_[i])->second;
        if (!peer::IsTerriblePeer(peer))
            out->emplace_back(peer.addr());
    }
    
    return true;
}

uint64_t Peers::MakeMapKey(const NetAddr& addr, bool by_group)
{
    crypto::HashOStream hs;
    
    if (by_group) {
        std::vector<uint8_t> group;        
        addr.GetGroup(&group);
        hs << key_ << group;
    }
    else {
        std::vector<uint8_t> vec_addr(kIpByteSize);
        for (int i = 0; i < kIpByteSize; i++)
            vec_addr.push_back(addr.GetByte(i));
        //std::vector<uint32_t> vec_addr;
        //for (int i = 0; i < NetAddr::ip_uint32_size; i++)
        //    vec_addr.push_back(addr.proto_addr().ip()[i]);
        hs << key_ << vec_addr;
    }
    
    return util::FromLittleEndian<uint64_t>(hs.Sha256().data());
}

bool Peers::SerializeToOstream(std::ostream * output) const
{
    LOCK(cs_peers_);
    return proto_peers_.SerializeToOstream(output);
}

bool Peers::ParseFromIstream(std::istream * input)
{
    LOCK(cs_peers_);
    return proto_peers_.ParseFromIstream(input);
}

void Peers::Clear()
{
    LOCK(cs_peers_);
    proto_peers_.mutable_new_tbl()->clear();
    proto_peers_.mutable_tried_tbl()->clear();
    proto_peers_.mutable_map_peers()->clear();
    rand_order_keys_.clear();
}

size_t Peers::Size() const
{
    LOCK(cs_peers_);
    return proto_peers_.map_peers().size();
}

bool Peers::IsEmpty() const
{
    LOCK(cs_peers_);
    return proto_peers_.map_peers().empty();
}

proto_peers::Peers Peers::proto_peers() const // thread safe copy
{
    LOCK(cs_peers_);
    return proto_peers_;
}

std::vector<uint64_t> Peers::rand_order_keys() const
{
    LOCK(cs_peers_);
    return rand_order_keys_;
}

util::Hash256 Peers::key() const
{
    return key_;
}

void Peers::EraseRand(uint64_t key)
{
    auto pos = rand_order_keys_.end();
    for (auto it = rand_order_keys_.begin(); it != rand_order_keys_.end(); ++it)
        if (key == *it) {
            pos = it;
            break;
        }
    
    if (pos != rand_order_keys_.end()) {
        std::swap(*pos, rand_order_keys_[rand_order_keys_.size()-1]);
        rand_order_keys_.pop_back();
    }
}

Peers& SingletonPeers::GetInstance()
{
    static Peers peers;
    return peers;
}

PeersDb::PeersDb(const fs::path& path)
    : path_peers_(path / default_peers_file)
{
}

bool PeersDb::DumpPeers(const Peers& peers)
{
    std::fstream fs(path_peers_, std::ios::out | std::ios::trunc | std::ios::binary);
    
    if (!peers.SerializeToOstream(&fs)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Flushing banned node to banlist.data failed.";
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Flushed " << peers.Size() << " to peers.dat";
    
    return true;
}

bool PeersDb::LoadPeers(Peers *peers)
{
    if (!peers)
        return false;
    
    peers->Clear();
    std::fstream fs(path_peers_, std::ios::in | std::ios::binary);
    if (!fs) {
        BTCLOG(LOG_LEVEL_INFO) << "Load "<< path_peers_  << ", but file not found.";
        return false;
    }
    
    return peers->ParseFromIstream(&fs);
}

const fs::path& PeersDb::path_peers() const
{
    return path_peers_;
}

} // namespace network
} // namespace btclite
