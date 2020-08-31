#ifndef BTCLITE_PROTOCOL_GETBLOCKS_H
#define BTCLITE_PROTOCOL_GETBLOCKS_H


#include "message.h"


namespace btclite {
namespace network {
namespace protocol {

class GetBlocks : public MessageData {
public:
    GetBlocks() = default;
    
    GetBlocks(ProtocolVersion version, const consensus::BlockLocator& hashes,
              const util::Hash256& hash_stop);    
    GetBlocks(ProtocolVersion version, consensus::BlockLocator&& hashes,
              const util::Hash256& hash_stop) noexcept;
    
    //-------------------------------------------------------------------------
    bool RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const;    
    std::string Command() const;
    bool IsValid() const;
    void Clear();
    size_t SerializedSize() const;
    util::Hash256 GetHash() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const GetBlocks& b) const;
    bool operator!=(const GetBlocks& b) const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    ProtocolVersion version() const;
    void set_version(ProtocolVersion version);
    
    const consensus::BlockLocator& hashes() const;
    void set_hashes(const consensus::BlockLocator& hashes);
    void set_hashes(consensus::BlockLocator&& hashes);
    
    const util::Hash256& hash_stop() const;
    void set_hash_stop(const util::Hash256& hash_stop);
    
private:
    ProtocolVersion version_ = kUnknownProtoVersion;
    consensus::BlockLocator hashes_;
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


} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_GETBLOCKS_H
