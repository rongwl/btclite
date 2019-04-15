#ifndef BTCLITE_MESSAGE_VERSION_H
#define BTCLITE_MESSAGE_VERSION_H

#include "message_types.h"

namespace btc_message {

class Version : public BaseMsgType {
public:
    static const std::string command;
    
    template <size_t N>
    struct RawNetData {
        int32_t value_;
        uint64_t services_;
        int64_t timestamp_;
        btc_message::NetAddr addr_recv_;
        btc_message::NetAddr addr_from_;
        uint64_t nonce_;
        VarStr<N, VarIntSize> user_agent_;
        int32_t start_height_;
        bool relay_;
    };
    
    //-------------------------------------------------------------------------
    Version()
        : value_(0), services_(0), timestamp_(0), address_receiver_(),
          address_from_(), nonce_(0), user_agent_(), start_height_(0),
          relay_(false) {}
    
    Version(uint32_t value, uint64_t services, uint64_t timestamp,
            const btclite::NetAddr& address_receiver,
            const btclite::NetAddr& address_from, uint64_t nonce,
            const std::string& user_agent, uint32_t start_height, bool relay)
        : value_(value), services_(services), timestamp_(timestamp),
          address_receiver_(address_receiver), address_from_(address_from),
          nonce_(nonce), user_agent_(user_agent), start_height_(start_height),
          relay_(relay) {}
    
    Version(uint32_t value, uint64_t services, uint64_t timestamp,
            btclite::NetAddr&& address_receiver, btclite::NetAddr&& address_from,
            uint64_t nonce, std::string&& user_agent, uint32_t start_height,
            bool relay)
        : value_(value), services_(services), timestamp_(timestamp),
          address_receiver_(std::move(address_receiver)),
          address_from_(std::move(address_from)),
          nonce_(nonce), user_agent_(std::move(user_agent)),
          start_height_(start_height), relay_(relay) {}
    
    Version(const Version& version)
        : Version(version.value_, version.services_, version.timestamp_,
                  version.address_receiver_, version.address_from_, version.nonce_,
                  version.user_agent_, version.start_height_, version.relay_) {}
    
    Version(Version&& version) noexcept
        : Version(version.value_, version.services_, version.timestamp_,
                  std::move(version.address_receiver_),
                  std::move(version.address_from_), version.nonce_,
                  std::move(version.user_agent_), version.start_height_,
                  version.relay_) {}
    
    //-------------------------------------------------------------------------
    bool IsValid();
    void Clear();
    
    //-------------------------------------------------------------------------
    Version& operator=(const Version& b);
    Version& operator=(Version&& b) noexcept;
    
    //-------------------------------------------------------------------------
    bool RecvMsgHandle();
    void GetRawData(const char *in);
    void SetRawData(char *out);
    
    //-------------------------------------------------------------------------
    uint32_t value() const
    {
        return value_;
    }
    void set_value(uint32_t value)
    {
        value_ = value;
    }

    uint64_t services() const
    {
        return services_;
    }
    void set_services(uint64_t services)
    {
        services_ = services;
    }

    uint64_t timestamp() const
    {
        return timestamp_;
    }
    void set_timestamp(uint64_t timestamp)
    {
        timestamp_ = timestamp;
    }
    
    const btclite::NetAddr& address_receiver() const
    {
        return address_receiver_;
    }
    void set_address_receiver(const btclite::NetAddr& addr)
    {
        address_receiver_ = addr;
    }
    void set_address_receiver(btclite::NetAddr&& addr) noexcept
    {
        address_receiver_ = std::move(addr);
    }

    const btclite::NetAddr& address_from() const
    {
        return address_from_;
    }
    void set_address_from(const btclite::NetAddr& addr)
    {
        address_from_ = addr;
    }
    void set_address_from(btclite::NetAddr&& addr) noexcept
    {
        address_from_ = std::move(addr);
    }
    
    uint64_t nonce() const
    {
        return nonce_;
    }
    void set_nonce(uint64_t nonce)
    {
        nonce_ = nonce;
    }

    const std::string& user_agent() const
    {
        return user_agent_;
    }
    void set_user_agent(const std::string& agent)
    {
        user_agent_ = agent;
    }
    void set_user_agent(std::string&& agent) noexcept
    {
        user_agent_ = std::move(agent);
    }

    uint32_t start_height() const
    {
        return start_height_;
    }
    void set_start_height(uint32_t height)
    {
        start_height_ = height;
    }

    bool relay() const
    {
        return relay_;
    }
    void set_relay(bool relay)
    {
        relay_ = relay;
    }

private:
    uint32_t value_;
    uint64_t services_;
    uint64_t timestamp_;
    btclite::NetAddr address_receiver_;
    
    // Fields below require version ≥ 106
    btclite::NetAddr address_from_;
    uint64_t nonce_;
    std::string user_agent_;
    uint32_t start_height_;
    
    // Fields below require version ≥ 70001
    bool relay_;
};

} // namespace btc_message

#endif // BTCLITE_MESSAGE_VERSION_H
