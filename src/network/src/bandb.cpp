#include "bandb.h"
#include "p2p.h"
#include "utility/include/logging.h"
#include "utiltime.h"


bool BanDb::Add(const btclite::NetAddr& addr, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch)
{
    SubNet subnet(addr);
    return Add(subnet, ban_reason, bantime_offset, since_unix_epoch);
}

bool BanDb::Add(const SubNet& sub_net, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch)
{
    proto_banmap::BanEntry ban_entry;
    
    ban_entry.set_create_time(GetTimeSeconds());
    ban_entry.set_ban_reason(ban_reason);
    if (bantime_offset <= 0)
        since_unix_epoch = false;
    int64_t ban_until = (since_unix_epoch ? 0 : GetTimeSeconds()) + bantime_offset;
    ban_entry.set_ban_until(ban_until);
    
    if (!Add_(sub_net, ban_entry))
        return false;
    
    P2P::mutable_nodes()->DisconnectBanNode(sub_net);
    
    if (ban_reason == ManuallyAdded)
        DumpBanlist();

    return true;
}

bool BanDb::Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry)
{
    LOCK(cs_banmap_);
    google::protobuf::Map< ::std::string, ::proto_banmap::BanEntry > *pmap = banmap_.mutable_map();
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
    
    LOCK(cs_banmap_);
    for (auto it = banmap_.map().begin(); it != banmap_.map().end(); ++it) {
        if (now > it->second.ban_until()) {
            banmap_.mutable_map()->erase(it->first);
            dirty_ = true;
            BTCLOG_MOD(LOG_LEVEL_INFO, Logging::NET) << "Removed banned node ip/subnet from banlist.dat: " << it->first;
        }
    }
}

void BanDb::DumpBanlist()
{
    SweepBanned(); // clean unused entries (if bantime has expired)
    
    if (!dirty())
        return;
}
