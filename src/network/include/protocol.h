#ifndef BTCLITE_PROTOCOL_H
#define BTCLITE_PROTOCOL_H

#include <array>
#include <cstdint>

#include "serialize.h"


/* Services flags */
enum ServiceFlags : uint64_t {
    // Nothing
    NODE_NONE = 0,
	
    // NODE_NETWORK means that the node is capable of serving the complete block chain. It is currently
    // set by all Bitcoin Core non pruned nodes, and is unset by SPV clients or other light clients.
    NODE_NETWORK = (1 << 0),
	
    // NODE_GETUTXO means the node is capable of responding to the getutxo protocol request.
    // Bitcoin Core does not support this but a patch set called Bitcoin XT does.
    // See BIP 64 for details on how this is implemented.
    NODE_GETUTXO = (1 << 1),
	
    // NODE_BLOOM means the node is capable and willing to handle bloom-filtered connections.
    // Bitcoin Core nodes used to support this by default, without advertising this bit,
    // but no longer do as of protocol version 70011 (= NO_BLOOM_VERSION)
    NODE_BLOOM = (1 << 2),
	
    // NODE_WITNESS indicates that a node can be asked for blocks and transactions including
    // witness data.
    NODE_WITNESS = (1 << 3),
	
    // NODE_XTHIN means the node supports Xtreme Thinblocks
    // If this is turned off then the node will not service nor make xthin requests
    NODE_XTHIN = (1 << 4),
	
    // NODE_NETWORK_LIMITED means the same as NODE_NETWORK with the limitation of only
    // serving the last 288 (2 day) blocks
    // See BIP159 for details on how this is implemented.
    NODE_NETWORK_LIMITED = (1 << 10),

    // Bits 24-31 are reserved for temporary experiments. Just pick a bit that
    // isn't getting used, or one not being used much, and notify the
    // bitcoin-development mailing list. Remember that service bits are just
    // unauthenticated advertisements, so your code must be robust against
    // collisions and other cases where nodes may be advertising a service they
    // do not actually support. Other service bits should be allocated via the
    // BIP process.
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
	
	MessageHeader(uint32_t magic, const std::string& command, uint32_t payload_length, uint32_t checksum)
		: magic_(magic), command_(command), payload_length_(payload_length), checksum_(checksum) {}
	
	MessageHeader(uint32_t magic, const std::string&& command, uint32_t payload_length, uint32_t checksum) noexcept
		: magic_(magic), command_(std::move(command)), payload_length_(payload_length), checksum_(checksum) {}
	
    MessageHeader(const MessageHeader& header)
		: MessageHeader(header.magic_, header.command_, header.payload_length_, header.checksum_) {}
	
	MessageHeader(const MessageHeader&& header) noexcept
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
	
	//-------------------------------------------------------------------------
	bool IsValid(uint32_t magic) const;
	
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


#endif // BTCLITE_PROTOCOL_H
