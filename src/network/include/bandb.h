#ifndef BTCLITE_BANDB_H
#define BTCLITE_BANDB_H

#include "banmap.pb.h"
#include "fs.h"
#include "network_address.h"
#include "sync.h"
#include "util.h"


class BanDb {
public:
    enum class BanReason {
        Unknown          = 0,
        NodeMisbehaving  = 1,
        ManuallyAdded    = 2
    };
    
    explicit BanDb(const fs::path& path)
        : path_ban_list_(path / default_ban_list), 
          ban_map_(), dirty_(false) {}
    
    BanDb(const fs::path& path, const proto_banmap::BanMap& ban_map)
        : path_ban_list_(path / default_ban_list),
          ban_map_(ban_map), dirty_(true) {}
    
    //-------------------------------------------------------------------------
    bool Add(const btclite::network::NetAddr& addr, const BanReason& ban_reason, bool dump_list = true);
    bool Add(const SubNet& sub_net, const BanReason& ban_reason, bool dump_list = true);
    bool Erase(const btclite::network::NetAddr& addr, bool dump_list = true);
    bool Erase(const SubNet& sub_net, bool dump_list = true);
    void Clear();
    
    size_t Size() const
    {
        LOCK(cs_ban_map_);
        return ban_map_.map().size();
    }
    
    //-------------------------------------------------------------------------
    void SweepBanned();
    bool DumpBanList();
    bool LoadBanList();
    bool IsBanned(btclite::network::NetAddr addr);
    
    //-------------------------------------------------------------------------
    const fs::path& path_ban_list() const
    {
        return path_ban_list_;
    }
    
    proto_banmap::BanMap ban_map() const // thread safe copy
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
    const std::string default_ban_list = "banlist.dat";
    
    fs::path path_ban_list_;
    mutable CriticalSection cs_ban_map_;
    proto_banmap::BanMap ban_map_;
    bool dirty_;
    
    bool Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry);
};

class SingletonBanDb : Uncopyable {
public:
    static BanDb& GetInstance(fs::path path = fs::path())
    {
        static BanDb ban_db(path);
        return ban_db;
    }
    
private:
    SingletonBanDb() {}
};

#endif // BTCLITE_NETWORK_BANDB_H
