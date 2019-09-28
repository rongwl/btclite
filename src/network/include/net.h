#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H


#include "network_address.h"
#include "protocol.h"
#include "serialize.h"
#include "thread.h"
#include "util.h"


using NodeId = int64_t;

class LocalNetConfig {
public:
    LocalNetConfig()
        : local_services_(ServiceFlags(kNodeNetwork | kNodeNetworkLimited)), local_addrs_() {}
    
    bool LookupLocalAddrs();
    bool IsLocal(const btclite::network::NetAddr& addr);
    
    ServiceFlags local_services() const
    {
        LOCK(cs_local_net_config_);
        return local_services_;
    }
    void set_local_services(ServiceFlags flags)
    {
        LOCK(cs_local_net_config_);
        local_services_ = flags;
    }
    
    std::vector<btclite::network::NetAddr> local_addrs() const // thread safe copy
    {
        LOCK(cs_local_net_config_);
        return local_addrs_;
    }
    
private:
    mutable CriticalSection cs_local_net_config_;
    ServiceFlags local_services_;
    std::vector<btclite::network::NetAddr> local_addrs_;
    
    bool AddLocalHost(const btclite::network::NetAddr& addr);
};

class SingletonLocalNetCfg : Uncopyable {
public:
    static LocalNetConfig& GetInstance()
    {
        static LocalNetConfig config;
        return config;
    }
    
private:
    SingletonLocalNetCfg() {}
};

class SingletonNetInterrupt : Uncopyable {
public:
    static ThreadInterrupt& GetInstance()
    {
        static ThreadInterrupt interrupt;
        return interrupt;
    }
    
private:
    SingletonNetInterrupt() {}
};


class MessageHeader {
public:
    static constexpr size_t kMessageStartSize = 4;
    static constexpr size_t kCommandSize = 12;
    static constexpr size_t kPayloadSize = 4;
    static constexpr size_t kChecksumSize = 4;
    static constexpr size_t kSize = kMessageStartSize + kCommandSize + kPayloadSize + kChecksumSize;
    
    using MsgMagic = uint32_t;
    
    //-------------------------------------------------------------------------
    MessageHeader()
        : magic_(0), command_(), payload_length_(0), checksum_(0)
    {
        std::memset(command_.data(), 0, kCommandSize);
    }
    
    explicit MessageHeader(uint32_t magic)
        : magic_(magic), command_(), payload_length_(0), checksum_(0) 
    {
        std::memset(command_.data(), 0, kCommandSize);
    }
    
    explicit MessageHeader(const uint8_t *raw_data)
        : magic_(0), command_(), payload_length_(0), checksum_(0)
    {
        std::memset(command_.data(), 0, kCommandSize);
        UnSerialize(raw_data);
    }
    
    MessageHeader(uint32_t magic, const std::string& command,
                  uint32_t payload_length, uint32_t checksum)
        : magic_(magic), command_(), payload_length_(payload_length), checksum_(checksum) 
    {
        std::memset(command_.data(), 0, kCommandSize);
        set_command(command);
    }
    
    MessageHeader(const MessageHeader& header)
        : magic_(header.magic_), command_(header.command_),
          payload_length_(header.payload_length_), checksum_(header.checksum_) {}
    
    MessageHeader(MessageHeader&& header) noexcept
        : magic_(header.magic_), command_(std::move(header.command_)),
          payload_length_(header.payload_length_), checksum_(header.checksum_) {}
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& out) const;
    template <typename Stream>
    void UnSerialize(Stream& in);
    
    //-------------------------------------------------------------------------
    bool IsValid() const;
    
    //-------------------------------------------------------------------------
    bool operator==(const MessageHeader& b) const
    {
        return (magic_ == b.magic_) &&
               (command_ == b.command_) &&
               (payload_length_ == b.payload_length_) &&
               (checksum_ == b.checksum_);
    }
    
    bool operator!=(const MessageHeader& b) const
    {
        return !(*this == b);
    }
    
    //-------------------------------------------------------------------------
    uint32_t magic() const
    {
        return magic_;
    }
    void set_magic(uint32_t magic)
    {
        magic_ = magic;
    }

    std::string command() const
    {
        const char *end = (const char*)std::memchr(command_.data(), '\0', kCommandSize);
        size_t size = end ? (end - command_.data()) : kCommandSize;
        return std::string(command_.data(), size);
    }
    
    void set_command(const std::string& command)
    {
        std::strncpy(command_.data(), command.data(), kCommandSize);
    }

    uint32_t payload_length() const
    {
        return payload_length_;
    }
    void set_payload_length(uint32_t payload_length)
    {
        payload_length_ = payload_length;
    }

    uint32_t checksum() const
    {
        return checksum_;
    }
    void set_checksum(uint32_t checksum)
    {
        checksum_ = checksum;
    }
    
private:    
    uint32_t magic_;
    std::array<char, kCommandSize> command_;
    uint32_t payload_length_;
    uint32_t checksum_;
};

template <typename Stream>
void MessageHeader::Serialize(Stream& out) const
{
    Serializer<Stream> serial(out);
    serial.SerialWrite(magic_);
    serial.SerialWrite(command_);
    serial.SerialWrite(payload_length_);
    serial.SerialWrite(checksum_);
}

template <typename Stream>
void MessageHeader::UnSerialize(Stream& in)
{
    /*if (!in)
        return;
    
    magic_ = *reinterpret_cast<const uint32_t*>(in);
    in += sizeof(magic_);
    
    std::memcpy(command_.data(), in, kCommandSize);
    in += kCommandSize;
    
    payload_length_ = *reinterpret_cast<const uint32_t*>(in);
    in += sizeof(payload_length_);
    
    checksum_ = *reinterpret_cast<const uint32_t*>(in);*/
}

class Message {
public:
    Message()
        : header_(), data_(nullptr) {}
    
    Message(const MessageHeader& header, const uint8_t *data_raw)
        : header_(header)
    {
        DataFactory(data_raw);
    }
    
    Message(MessageHeader&& header, const uint8_t *data_raw) noexcept
        : header_(std::move(header))
    {
        DataFactory(data_raw);
    }
    
    explicit Message(const uint8_t *raw)
        : header_(std::move(MessageHeader(raw)))
    {
        DataFactory(raw+MessageHeader::kSize);
    }
    
    //-------------------------------------------------------------------------
    void DataFactory(const uint8_t *raw);
    
    //-------------------------------------------------------------------------
    const MessageHeader& header() const
    {
        return header_;
    }
    
private:
    MessageHeader header_;
    std::shared_ptr<void> data_;
};

class NetArgs {
public:
    explicit NetArgs(const Args& args);
    
    bool is_listen() const
    {
        return is_listen_;
    }
    
    bool is_discover() const
    {
        return is_discover_;
    }
    
    bool is_dnsseed() const
    {
        return is_dnsseed_;
    }
    
    const std::vector<std::string>& specified_outgoing() const
    {
        return specified_outgoing_;
    }
    
private:
    bool is_listen_;
    bool is_discover_;
    bool is_dnsseed_;
    std::vector<std::string> specified_outgoing_;
};


#endif // BTCLITE_NET_H
