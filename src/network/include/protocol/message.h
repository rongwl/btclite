#ifndef BTCLITE_PROTOCOL_MESSAGE_H
#define BTCLITE_PROTOCOL_MESSAGE_H


#include "hash.h"
#include "node.h"
#include "network/include/params.h"


namespace btclite {
namespace network {
namespace protocol {

class MessageHeader {
public:
    static constexpr size_t kMessageStartSize = 4;
    static constexpr size_t kCommandSize = 12;
    static constexpr size_t kPayloadSize = 4;
    static constexpr size_t kChecksumSize = 4;
    static constexpr size_t kSize = kMessageStartSize + kCommandSize
                                    + kPayloadSize + kChecksumSize;
    
    MessageHeader() = default;
    
    MessageHeader(uint32_t magic, const std::string& command,
                  uint32_t payload_length, uint32_t checksum);    
    MessageHeader(uint32_t magic, std::string&& command,
                  uint32_t payload_length, uint32_t checksum) noexcept;
    
    explicit MessageHeader(const uint8_t *raw);
    
    //-------------------------------------------------------------------------
    bool IsValid(uint32_t magic = 0) const;
    void Clear();
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void Deserialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool operator==(const MessageHeader& b) const;    
    bool operator!=(const MessageHeader& b) const;
    
    //-------------------------------------------------------------------------
    uint32_t magic() const;
    void set_magic(uint32_t magic);

    std::string command() const;
    
    void set_command(const std::string& command);
    void set_command(std::string&& command) noexcept;

    uint32_t payload_length() const;
    void set_payload_length(uint32_t payload_length);

    uint32_t checksum() const;
    void set_checksum(uint32_t checksum);
    
private:    
    uint32_t magic_ = 0;
    std::array<char, kCommandSize> command_ = {};
    uint32_t payload_length_ = 0;
    uint32_t checksum_ = 0;
};

template <typename Stream>
void MessageHeader::Serialize(Stream& out) const
{
    util::Serializer<Stream> serializer(out);
    serializer.SerialWrite(magic_);
    serializer.SerialWrite(command_);
    serializer.SerialWrite(payload_length_);
    serializer.SerialWrite(checksum_);
}

template <typename Stream>
void MessageHeader::Deserialize(Stream& in)
{
    util::Deserializer<Stream> deserializer(in);
    deserializer.SerialRead(&magic_);
    deserializer.SerialRead(&command_);
    deserializer.SerialRead(&payload_length_);
    deserializer.SerialRead(&checksum_);
}

class MessageData {
public:
    virtual bool RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const = 0;
    virtual std::string Command() const = 0;
    virtual size_t SerializedSize() const = 0;
    virtual bool IsValid() const = 0;
    virtual ~MessageData() {}
    
    static const std::string kCommand;
};

bool CheckMisbehaving(const std::string command, std::shared_ptr<Node> src_node);

} // namespace protocol
} // namespace network
} // namespace btclite


#endif // BTCLITE_MESSAGE_TYPES_MESSAGE_H
