#ifndef BTCLITE_NETWORK_PARAMS_H
#define BTCLITE_NETWORK_PARAMS_H

#include "environment.h"
#include "protocol.h"

struct Seed {
	std::string host_;
	uint16_t prot_;
};

struct Seed6 {
    uint8_t addr_[16];
    uint16_t port_;
};

namespace Network {

class Params {
public:
	void Init(NetworkEnv env);
	
	//-------------------------------------------------------------------------
	MessageHeader::MsgMagic msg_magic()
	{
		return msg_magic_;
	}
	
	uint16_t default_port()
	{
		return default_port_;
	}
	
	const std::vector<Seed>& seeds() const
	{
		return seeds_;
	}
	
	const std::vector<Seed6>& seeds6() const
	{
		return seeds6_;
	}
	
private:
	MessageHeader::MsgMagic msg_magic_;
	uint16_t default_port_;
	std::vector<Seed> seeds_;
	std::vector<Seed6> seeds6_;
};

} // namespace Network

#endif // BTCLITE_NETWORK_PARAMS_H
