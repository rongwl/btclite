#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H


#include "environment.h"
#include "network_address.h"
#include "serialize.h"
#include "util.h"

#include "message_types/version.h"

    
class LocalNetConfig {
public:
    LocalNetConfig()
        : local_services_(ServiceFlags(NODE_NETWORK | NODE_NETWORK_LIMITED)), local_addrs_() {}
    
    void LookupLocalAddrs();
    
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
    
    /*const std::vector<btclite::NetAddr>& local_addrs() const
    {
        LOCK(cs_local_net_config_);
        return local_addrs_;
    }*/
    
private:
    mutable CriticalSection cs_local_net_config_;
    ServiceFlags local_services_;
    std::vector<btclite::NetAddr> local_addrs_;
    
    bool AddLocalHost(const btclite::NetAddr& addr);
};


class MessageHeader {
public:
    static constexpr size_t MESSAGE_START_SIZE = 4;
    static constexpr size_t COMMAND_SIZE = 12;
    static constexpr size_t CHECKSUM_SIZE = 4;
    
    struct RawNetData {
        char magic_[MESSAGE_START_SIZE];
        char command_[COMMAND_SIZE];
        uint32_t payload_length_;
        uint8_t checksum_[CHECKSUM_SIZE];
    };    
    using MsgMagic = uint32_t;
    
    //-------------------------------------------------------------------------
    MessageHeader()
        : magic_(0), command_(), payload_length_(0), checksum_(0) {}
    
    explicit MessageHeader(uint32_t magic)
        : magic_(magic), command_(), payload_length_(0), checksum_(0) {}
    
    explicit MessageHeader(const char *raw_data)
    {
        GetRawData(raw_data);
    }
    
    MessageHeader(uint32_t magic, const std::string& command, uint32_t payload_length, uint32_t checksum)
        : magic_(magic), command_(command), payload_length_(payload_length), checksum_(checksum) {}
    
    MessageHeader(uint32_t magic, std::string&& command, uint32_t payload_length, uint32_t checksum) noexcept
        : magic_(magic), command_(std::move(command)), payload_length_(payload_length), checksum_(checksum) {}
    
    MessageHeader(const MessageHeader& header)
        : MessageHeader(header.magic_, header.command_, header.payload_length_, header.checksum_) {}
    
    MessageHeader(MessageHeader&& header) noexcept
        : MessageHeader(header.magic_, std::move(header.command_), header.payload_length_, header.checksum_) {}

    
    //-------------------------------------------------------------------------
    template <typename Stream> void Serialize(Stream& os) const
    {
        Serializer<Stream> serial(os);
        serial.SerialWrite(magic_);
        serial.SerialWrite(command_);
        serial.SerialWrite(payload_length_);
        serial.SerialWrite(checksum_);
    }
    template <typename Stream> void UnSerialize(Stream& is)
    {
        Serializer<Stream> serial(is);
        serial.SerialRead(&magic_);
        serial.SerialRead(&command_);
        serial.SerialRead(&payload_length_);
        serial.SerialRead(&checksum_);
    }
    bool GetRawData(const char *in)
    {
        
    }
    void SetRawData(char *cout)
    {
        
    }
    
    //-------------------------------------------------------------------------
    bool IsValid(BaseEnv env) const;
    
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

    const std::string& command() const
    {
        return command_;
    }
    void set_command(const std::string& command)
    {
        command_ = command;
    }
    void set_command(const std::string&& command) noexcept
    {
        command_ = std::move(command);
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
    std::string command_;
    uint32_t payload_length_;
    uint32_t checksum_;
};

class Message {
public:
    Message()
        : header_(), data_(nullptr) {}
    
    explicit Message(const MessageHeader& header)
        : header_(header)
    {
        DataFactory(header.command());
    }
    
    explicit Message(const char *raw_data)
        : Message(MessageHeader(raw_data)) {}
    
    //-------------------------------------------------------------------------
    void DataFactory(const std::string& command);
    bool RecvMsgHandle();
    
private:
    MessageHeader header_;
    std::shared_ptr<btc_message::BaseMsgType> data_;
};


struct NetArgs {
    bool is_listen_;
    bool is_discover_;
    bool is_dnsseed_;
    std::vector<std::string> specified_outgoing_;
};

#endif // BTCLITE_NET_H
