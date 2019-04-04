#ifndef BTCLITE_P2P_H
#define BTCLITE_P2P_H

#include <thread>

#include "hash.h"
#include "protocol.h"
#include "net.h"
#include "network/include/params.h"
#include "thread.h"

class P2P {
public:
	bool Init(NetworkEnv env);
	bool Start();
	bool Interrupt();
	bool Stop();
	
	//-------------------------------------------------------------------------
	const Network::Params& network_params() const
	{
		return network_params_;
	}
	
private:
	Network::Params network_params_;
	LocalNetConfig local_network_config_;
	
	ThreadInterrupt interrupt_;
	std::thread thread_dns_seeds_;
    std::thread thread_socket_handler_;
    std::thread thread_open_connections_;
    std::thread thread_message_handler_;
	
	std::vector<std::string> m_specified_outgoing;
	
	void ThreadDnsSeeds();
};

#endif // BTCLITE_P2P_H
