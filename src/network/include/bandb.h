#ifndef BTCLITE_BANDB_H
#define BTCLITE_BANDB_H

#include "banmap.pb.h"
#include "fs.h"
#include "network_address.h"
#include "sync.h"

#define DEFAULT_BAN_LIST "banlist.dat"


class BanDb {
public:
    enum BanReason {
        Unknown          = 0,
        NodeMisbehaving  = 1,
        ManuallyAdded    = 2
    };
    
    BanDb()
        : path_ban_list_(), ban_map_(), dirty_(false) {}
    
    bool Init(const fs::path& path_data_dir)
    {
        path_ban_list_ = path_data_dir / DEFAULT_BAN_LIST;
        return true;
    }
    
    bool Add(const btclite::NetAddr& addr, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch);
    bool Add(const SubNet& sub_net, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch);
    
    //-------------------------------------------------------------------------
    void SweepBanned();
    void DumpBanlist();
    
    //-------------------------------------------------------------------------
    const fs::path& path_ban_list() const
    {
        return path_ban_list_;
    }
    
    proto_banmap::BanMap ban_map() // thread safe copy
    {
        LOCK(cs_ban_map_);
        return ban_map_;
    }
    
    bool dirty() const
    {
        LOCK(cs_ban_map_);
        return dirty_;
    }
    
    void set_dirty(bool dirty)
    {
        LOCK(cs_ban_map_);
        dirty_ = dirty;
    }
    
private:
    fs::path path_ban_list_;
    mutable CriticalSection cs_ban_map_;
    proto_banmap::BanMap ban_map_;
    bool dirty_;
    
    bool Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry);
};

#endif // BTCLITE_NETWORK_BANDB_H
