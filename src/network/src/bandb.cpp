#include "bandb.h"
#include "node.h"
#include "utiltime.h"


bool BanDb::Add(const btclite::NetAddr& addr, const BanReason &ban_reason, bool dump_list)
{
    return Add(SubNet(addr), ban_reason, dump_list);
}

bool BanDb::Add(const SubNet& sub_net, const BanReason &ban_reason, bool dump_list)
{
    proto_banmap::BanEntry ban_entry;
    
    ban_entry.set_create_time(GetTimeSeconds());
    ban_entry.set_ban_reason(ban_reason);
    ban_entry.set_ban_until(GetTimeSeconds() + default_misbehaving_bantime);
    
    if (!Add_(sub_net, ban_entry))
        return false;
    
    SingletonNodes::GetInstance().DisconnectBanNode(sub_net);
    
    if (ban_reason == ManuallyAdded && dump_list)
        DumpBanList();

    return true;
}

bool BanDb::Erase(const btclite::NetAddr& addr, bool dump_list)
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
    ban_map_.clear_map();
    dirty_ = true;
    DumpBanList();    
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
    int64_t now = GetTimeSeconds();
    
    LOCK(cs_ban_map_);
    for (auto it = ban_map_.map().begin(); it != ban_map_.map().end(); ++it) {
        if (now > it->second.ban_until()) {
            ban_map_.mutable_map()->erase(it->first);
            dirty_ = true;
            BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "Removed banned node ip/subnet from banlist.dat: " << it->first;
        }
    }
}

void BanDb::DumpBanList()
{
    SweepBanned(); // clean unused entries (if bantime has expired)
    
    if (!dirty())
        return;
    
    proto_banmap::BanMap banmap = ban_map();
    std::fstream fs(path_ban_list_, std::ios::out | std::ios::trunc | std::ios::binary);
    if (banmap.SerializeToOstream(&fs))
        set_dirty(false);
    
    BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "Flushed " << banmap.map().size() << " banned node ips/subnets to banlist.dat";
}

bool BanDb::IsBanned(btclite::NetAddr addr)
{
    LOCK(cs_ban_map_);
    
    for (auto it = ban_map_.map().begin(); it != ban_map_.map().end(); ++it) {
        if (SubNet(addr).ToString() == it->first && GetTimeSeconds() < it->second.ban_until()) {
            //std::cout << "map str:" << it->first << " subnet str:" << SubNet(addr).ToString() << '\n';
            return true;
        }
    }
    
    return false;
}
