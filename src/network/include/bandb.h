#ifndef BTCLITE_BANDB_H
#define BTCLITE_BANDB_H

#include "banmap.pb.h"
#include "network_address.h"
#include "sync.h"


class BanDb {
public:
    enum BanReason {
        Unknown          = 0,
        NodeMisbehaving  = 1,
        ManuallyAdded    = 2
    };
    
    bool Add(const btclite::NetAddr& addr, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch);
    bool Add(const SubNet& sub_net, const BanReason &ban_reason, int64_t bantime_offset, bool since_unix_epoch);
    
    
    void SweepBanned();
    void DumpBanlist();
    
    //-------------------------------------------------------------------------
    bool dirty() const
    {
        LOCK(cs_banmap_);
        return dirty_;
    }
    
    void set_dirty(bool dirty)
    {
        LOCK(cs_banmap_);
        dirty_ = dirty;
    }
    
private:
    mutable CriticalSection cs_banmap_;
    proto_banmap::BanMap banmap_;
    bool dirty_;
    
    bool Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry);
};

#endif // BTCLITE_NETWORK_BANDB_H
