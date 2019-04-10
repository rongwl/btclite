#ifndef BTCLITE_MESSAGE_TYPES_H
#define BTCLITE_MESSAGE_TYPES_H

#include <cstdint>
#include <string>

#include "network_address.pb.h"
#include "network_address.h"
#include "protocol.h"


namespace btc_message {

struct NetAddr {
	uint32_t time_;
	uint64_t serviecs_;
	char ip_[16];
	uint16_t port_;
};

template <size_t N>
struct VarInt {
	uint8_t value_[N];
};

template <size_t N, size_t (*F)(size_t)>
struct VarStr {
	VarInt<F(N)> length_;
	char string_[N];
};

class BaseMsgType {
public:
	virtual bool RecvMsgHandle() = 0;
	virtual void GetRawData(const char *in) = 0;
	virtual void SetRawData(char *out) = 0;
};

} // namespace message

#endif // BTCLITE_MESSAGE_TYPES_H
