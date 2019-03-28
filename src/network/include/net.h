#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H

#include <cstdint>
#include <cstring>
#include <string>
#include <utility>

#include "serialize.h"

class MessageHeader {
public:
	static constexpr std::size_t COMMAND_SIZE = 12;
	using MsgMagic = uint32_t;
	
	MessageHeader()
		: magic_(0), command_(), payload_length_(0), checksum_(0) {}
	
	explicit MessageHeader(uint32_t magic)
		: magic_(magic), command_(), payload_length_(0), checksum_(0) {}
	MessageHeader(uint32_t magic, const std::array<char, COMMAND_SIZE>& command,
				  uint32_t payload_length, uint32_t checksum)
		: magic_(magic), command_(command), payload_length_(payload_length), checksum_(checksum) {}
	MessageHeader(uint32_t magic, const std::array<char, COMMAND_SIZE>&& command,
				  uint32_t payload_length, uint32_t checksum) noexcept
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

    std::string command() const
	{
		std::size_t size = (std::strlen(command_.data()) < COMMAND_SIZE) ? std::strlen(command_.data()) : COMMAND_SIZE;
		return std::string(command_.data(), size);
	}
    void set_command(const std::array<char, COMMAND_SIZE>& command)
	{
		command_ = command;
	}
	void set_command(const std::array<char, COMMAND_SIZE>&& command) noexcept
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
    std::array<char, COMMAND_SIZE> command_;
    uint32_t payload_length_;
    uint32_t checksum_;
};

#endif // BTCLITE_NET_H
