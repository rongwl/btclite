#ifndef BTCLITE_PROTOCOL_GETBLOCKS_H
#define BTCLITE_PROTOCOL_GETBLOCKS_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

namespace private_getblocks {

class GetBlocks : public MessageData {
public:
    GetBlocks() = default;
    
    GetBlocks(ProtocolVersion version, const chain::BlockLocator& hashes,
              const util::Hash256& hash_stop)
        : version_(version), hashes_(hashes), hash_stop_(hash_stop) {}
    
    GetBlocks(ProtocolVersion version, chain::BlockLocator&& hashes,
              const util::Hash256& hash_stop) noexcept
        : version_(version), hashes_(std::move(hashes)), 
          hash_stop_(hash_stop) {}
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node) const;
    
    std::string Command() const
    {
        return msg_command::kMsgGetBlocks;
    }
    
    bool IsValid() const
    {
        return (version_ != kUnknownProtoVersion && 
                (!hashes_.empty() || hashes_.size() > kMaxBlockLoactorSize));
    }
    
    void Clear() 
    {
        version_ = kUnknownProtoVersion;
        hashes_.clear();
        hash_stop_.Clear();
    }
    
    size_t SerializedSize() const
    {
        return sizeof(version_) + util::VarIntSize(hashes_.size()) +
               hashes_.size()*kHashSize + hash_stop_.Size();
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const GetBlocks& b) const
    {
        return (version_ == b.version_ && hashes_ == b.hashes_ &&
                hash_stop_ == b.hash_stop_);
    }
    
    bool operator!=(const GetBlocks& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    ProtocolVersion version() const
    {
        return version_;
    }
    
    void set_version(ProtocolVersion version)
    {
        version_ = version;
    }
    
    const chain::BlockLocator& hashes() const
    {
        return hashes_;
    }
    
    void set_hashes(const chain::BlockLocator& hashes)
    {
        hashes_ = hashes;
    }
    
    void set_hashes(chain::BlockLocator&& hashes)
    {
        hashes_ = std::move(hashes);
    }
    
    const util::Hash256& hash_stop() const
    {
        return hash_stop_;
    }
    
    void set_hash_stop(const util::Hash256& hash_stop)
    {
        hash_stop_ = hash_stop;
    }
    
private:
    ProtocolVersion version_ = kUnknownProtoVersion;
    chain::BlockLocator hashes_;
    util::Hash256 hash_stop_;
};

template <typename Stream>
void GetBlocks::Serialize(Stream& out) const
{
    util::Serializer<Stream> serializer(out);
    
    serializer.SerialWrite(version_);
    serializer.SerialWrite(hashes_);
    serializer.SerialWrite(hash_stop_);
}

template <typename Stream>
void GetBlocks::Deserialize(Stream& in)
{
    util::Deserializer<Stream> deserializer(in);
    
    deserializer.SerialRead(&version_);
    deserializer.SerialRead(&hashes_);
    deserializer.SerialRead(&hash_stop_);
}

} // namespace private_getblocks

using GetBlocks = crypto::Hashable<private_getblocks::GetBlocks>;

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_GETBLOCKS_H
