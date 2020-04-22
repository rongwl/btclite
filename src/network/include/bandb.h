#ifndef BTCLITE_BANDB_H
#define BTCLITE_BANDB_H

#include "banmap.pb.h"
#include "fs.h"
#include "network_address.h"
#include "sync.h"
#include "util.h"


namespace btclite {
namespace network {

class BanList {
public:
    enum class BanReason {
        kUnknown          = 0,
        kNodeMisbehaving  = 1,
        kManuallyAdded    = 2
    };
    
    BanList() = default;
    
    BanList(const proto_banmap::BanMap& ban_map)
        : ban_map_(ban_map) {}
    
    //-------------------------------------------------------------------------
    bool Add(const NetAddr& addr, const BanReason& ban_reason);
    bool Add(const SubNet& sub_net, const BanReason& ban_reason);
    bool Erase(const NetAddr& addr);
    bool Erase(const SubNet& sub_net);
    
    void Clear()
    {
        LOCK(cs_ban_map_);
        ban_map_.clear_map();
    }
    
    size_t Size() const
    {
        LOCK(cs_ban_map_);
        return ban_map_.map().size();
    }
    
    bool IsEmpty() const
    {
        LOCK(cs_ban_map_);
        return ban_map_.map().empty();
    }
    
    //-------------------------------------------------------------------------
    void SweepBanned();
    bool IsBanned(NetAddr addr);
    
    //-------------------------------------------------------------------------
    bool SerializeToOstream(std::ostream *output) const
    {
        LOCK(cs_ban_map_);
        return ban_map_.SerializeToOstream(output);
    }
    
    bool ParseFromIstream(std::istream *input)
    {
        LOCK(cs_ban_map_);
        return ban_map_.ParseFromIstream(input);
    }
    
    //-------------------------------------------------------------------------    
    proto_banmap::BanMap ban_map() const // thread safe copy
    {
        LOCK(cs_ban_map_);
        return ban_map_;
    }
    
private:
    mutable util::CriticalSection cs_ban_map_;
    proto_banmap::BanMap ban_map_;
    
    bool Add_(const SubNet& sub_net, const proto_banmap::BanEntry& ban_entry);
};

class SingletonBanList : util::Uncopyable {
public:
    static BanList& GetInstance()
    {
        static BanList ban_list;
        return ban_list;
    }
    
private:
    SingletonBanList() {}
};

class BanDb {
public:
    explicit BanDb(const fs::path& path)
        : path_ban_list_(path / default_ban_list) {}
    
    //-------------------------------------------------------------------------
    bool DumpBanList(BanList& ban_list);
    bool LoadBanList(BanList *ban_list);
    
    //-------------------------------------------------------------------------
    const fs::path& path_ban_list() const
    {
        return path_ban_list_;
    }
    
private:
    const std::string default_ban_list = "banlist.dat";
    
    fs::path path_ban_list_;
};

} //namespace network
} //namespace btclite

#endif // BTCLITE_NETWORK_BANDB_H
