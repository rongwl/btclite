#include "bandb.h"
#include "node.h"
#include "utiltime.h"


bool BanDb::Add(const btclite::network::NetAddr& addr, const BanReason& ban_reason, bool dump_list)
{
    return Add(SubNet(addr), ban_reason, dump_list);
}

bool BanDb::Add(const SubNet& sub_net, const BanReason& ban_reason, bool dump_list)
{
    proto_banmap::BanEntry ban_entry;
    
    ban_entry.set_create_time(btclite::utility::util_time::GetTimeSeconds());
    ban_entry.set_ban_reason(static_cast<std::underlying_type_t<BanReason> >(ban_reason));
    ban_entry.set_ban_until(btclite::utility::util_time::GetTimeSeconds() + kDefaultMisbehavingBantime);
    
    if (!Add_(sub_net, ban_entry))
        return false;
    
    SingletonNodes::GetInstance().DisconnectNode(sub_net);
    
    if (ban_reason == BanReason::ManuallyAdded && dump_list)
        DumpBanList();

    return true;
}

bool BanDb::Erase(const btclite::network::NetAddr& addr, bool dump_list)
{
    return Erase(SubNet(addr), dump_list);
}

bool BanDb::Erase(const SubNet& sub_net, bool dump_list)
{
    LOCK(cs_ban_map_);
    if (!ban_map_.mutable_map()->erase(sub_net.ToString()))
        return false;
    dirty_ = true;
    
    if (dump_list)
        DumpBanList();
    
    return true;
}

void BanDb::Clear()
{
    LOCK(cs_ban_map_);
    dirty_ = true;
    DumpBanList(); 
    ban_map_.clear_map();
}

bool BanDb::Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry)
{
    LOCK(cs_ban_map_);
    google::protobuf::Map< ::std::string, ::proto_banmap::BanEntry > *pmap = ban_map_.mutable_map();
    if ((*pmap)[sub_net.ToString()].ban_until() < ban_entry.ban_until()) {        
        (*pmap)[sub_net.ToString()] = ban_entry;
        dirty_ = true;
        return true;
    }
    
    return false;
}

void BanDb::SweepBanned()
{
    int64_t now = btclite::utility::util_time::GetTimeSeconds();
    
    LOCK(cs_ban_map_);
    for (auto it = ban_map_.map().begin(); it != ban_map_.map().end(); ++it) {
        if (now > it->second.ban_until()) {
            ban_map_.mutable_map()->erase(it->first);
            dirty_ = true;
            BTCLOG(LOG_LEVEL_VERBOSE) << "Removed banned node ip/subnet from banlist.dat: " << it->first;
        }
    }
}

bool BanDb::DumpBanList()
{
    SweepBanned(); // clean unused entries (if bantime has expired)
    
    if (!dirty())
        return false;
    
    proto_banmap::BanMap banmap = ban_map();
    std::fstream fs(path_ban_list_, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!banmap.SerializeToOstream(&fs)) {
        BTCLOG(LOG_LEVEL_ERROR) << "Flushing banned node to banlist.data failed.";
        return false;
    }
    set_dirty(false);
    
    BTCLOG(LOG_LEVEL_INFO) << "Flushed " << banmap.map().size() << " banned node ips/subnets to banlist.dat.";
    
    return true;
}

bool BanDb::LoadBanList()
{
    LOCK(cs_ban_map_);
    
    if (!ban_map_.map().empty())
        return false;
    
    std::fstream fs(path_ban_list_, std::ios::in | std::ios::binary);
    if (!fs) {
        BTCLOG(LOG_LEVEL_INFO) << "Load "<< path_ban_list_  << ", but file not found.";
        return false;
    }
    return ban_map_.ParseFromIstream(&fs);
}

bool BanDb::IsBanned(btclite::network::NetAddr addr)
{
    LOCK(cs_ban_map_);
    
    for (auto it = ban_map_.map().begin(); it != ban_map_.map().end(); ++it) {
        if (SubNet(addr).ToString() == it->first && btclite::utility::util_time::GetTimeSeconds() < it->second.ban_until()) {
            return true;
        }
    }
    
    return false;
}
