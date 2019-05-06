#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H

#include <cstring>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "environment.h"
#include "network_address.h"
#include "sync.h"
#include "util.h"

#include "message_types/version.h"


using Socket = int;

    
class LocalNetConfig {
public:
    LocalNetConfig()
        : local_services_(NODE_NONE), local_addrs_() {}
    
    void LookupLocalAddrs();
    
    ServiceFlags local_services() const
    {
        return local_services_;
    }
    void set_local_services(ServiceFlags flags)
    {
        local_services_ = flags;
    }
    
    const std::vector<btclite::NetAddr>& local_addrs() const
    {
        return local_addrs_;
    }
    
private:
    ServiceFlags local_services_;
    std::vector<btclite::NetAddr> local_addrs_;
    
    bool AddLocalAddr();
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


class BaseSocket {
public:
    Socket Create();
    bool Close();

    Socket socket_fd() const
    {
        return socket_fd_;
    }

protected:
    BaseSocket() {}
    
private:
    Socket socket_fd_;
};

// outbound socket connection
class Connector : public BaseSocket {
public:
    bool Connect();

};

// inbound socket connection
class Acceptor : public BaseSocket {
public:
    bool Bind();
    Socket Accept();
};


/* Information about a connected peer */
class BaseNode {
public:
    using NodeId = int64_t;
    
    void Connect();
    void Disconnect();
    size_t Receive();
    size_t Send();
    
    NodeId id() const
    {
        return id_;
    }
private:
    NodeId id_;
};
// mixin uncopyable
using Node = Uncopyable<BaseNode>;

class OutboundNode : public Node {
public:
private:
    Connector connector_;
    std::deque<std::vector<unsigned char> > send_msg_;
};

class InboundNode: public Node {
public:
private:
    Acceptor acceptor_;
    std::list<Message> recv_msg_;
};


class OutboundSession {
public:
    void GetAddrFromSeed();
    
private:
    std::unique_ptr<Semaphore> semaphore_;
    OutboundNode nodes_;
    
    void OpenConnection();
}; 

class InboundSession {
public:
private:
    InboundNode nodes_;
};


#endif // BTCLITE_NET_H
