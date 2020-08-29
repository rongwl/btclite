#include "banlist.h"
#include "node.h"
#include "util_time.h"


namespace btclite {
namespace network {

BanList::BanList(const proto_banmap::BanMap& ban_map)
    : ban_map_(ban_map) 
{
}

bool BanList::Add(const NetAddr& addr, const BanReason& ban_reason)
{
    return Add(SubNet(addr), ban_reason);
}

bool BanList::Add(const SubNet& sub_net, const BanReason& ban_reason)
{
    proto_banmap::BanEntry ban_entry;
    
    ban_entry.set_create_time(util::GetTimeSeconds());
    ban_entry.set_ban_reason(static_cast<std::underlying_type_t<BanReason> >(ban_reason));
    ban_entry.set_ban_until(util::GetTimeSeconds() + kDefaultMisbehavingBantime);
    
    if (!Add_(sub_net, ban_entry))
        return false;

    return true;
}

bool BanList::Erase(const NetAddr& addr)
{
    return Erase(SubNet(addr));
}

bool BanList::Erase(const SubNet& sub_net)
{
    LOCK(cs_ban_map_);
    if (!ban_map_.mutable_map()->erase(sub_net.ToString()))
        return false;
    
    return true;
}

void BanList::Clear()
{
    LOCK(cs_ban_map_);
    ban_map_.clear_map();
}

size_t BanList::Size() const
{
    LOCK(cs_ban_map_);
    return ban_map_.map().size();
}

bool BanList::IsEmpty() const
{
    LOCK(cs_ban_map_);
    return ban_map_.map().empty();
}

bool BanList::Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry)
{
    LOCK(cs_ban_map_);
    auto pmap = ban_map_.mutable_map();
    if ((*pmap)[sub_net.ToString()].ban_until() < ban_entry.ban_until()) {        
        (*pmap)[sub_net.ToString()] = ban_entry;
        return true;
    }
    
    return false;
}

void BanList::SweepBanned()
{
    int64_t now = util::GetTimeSeconds();
    
    LOCK(cs_ban_map_);
    for (auto it = ban_map_.map().begin(); it != ban_map_.map().end(); ++it) {
        if (now > it->second.ban_until()) {
            ban_map_.mutable_map()->erase(it->first);
            BTCLOG(LOG_LEVEL_VERBOSE) << "Removed banned node ip/subnet from banlist.dat: " 
                                      << it->first;
        }
    }
}

bool BanList::IsBanned(NetAddr addr) const
{
    LOCK(cs_ban_map_);
    
    auto it = ban_map_.map().find(SubNet(addr).ToString());
    if (it != ban_map_.map().end()) {
        if (util::GetTimeSeconds() < it->second.ban_until()) {
            return true;
        }
    }
    
    return false;
}

bool BanList::SerializeToOstream(std::ostream *output) const
{
    LOCK(cs_ban_map_);
    return ban_map_.SerializeToOstream(output);
}

bool BanList::ParseFromIstream(std::istream *input)
{
    LOCK(cs_ban_map_);
    return ban_map_.ParseFromIstream(input);
}

proto_banmap::BanMap BanList::ban_map() const // thread safe copy
{
    LOCK(cs_ban_map_);
    return ban_map_;
}

BanList& SingletonBanList::GetInstance()
{
    static BanList ban_list;
    return ban_list;
}

bool BanDb::DumpBanList(BanList& ban_list)
{
    ban_list.SweepBanned(); // clean unused entries (if bantime has expired)   
    
    std::fstream fs(path_ban_list_, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!ban_list.SerializeToOstream(&fs)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Flushing banned node to banlist.data failed.";
        return false;
    }
    
    BTCLOG(LOG_LEVEL_INFO) << "Flushed " << ban_list.Size() 
                           << " banned node ips/subnets to banlist.dat.";
    
    return true;
}

bool BanDb::LoadBanList(BanList *ban_list)
{
    if (!ban_list)
        return false;
    
    ban_list->Clear();
    std::fstream fs(path_ban_list_, std::ios::in | std::ios::binary);
    if (!fs) {
        BTCLOG(LOG_LEVEL_INFO) << "Load "<< path_ban_list_  << ", but file not found.";
        return false;
    }
    return ban_list->ParseFromIstream(&fs);
}

const fs::path& BanDb::path_ban_list() const
{
    return path_ban_list_;
}


} // namespace network
} // namespace btclite
