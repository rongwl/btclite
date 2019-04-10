#ifndef BTCLITE_NET_H
#define BTCLITE_NET_H

#include <cstring>
#include <deque>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <utility>

#include "network_address.pb.h"
#include "sync.h"
#include "util.h"

#include "message_types/version.h"


using Socket = int;

	
class LocalNetConfig {
public:
	void LookupLocalAddrs();
	
	ServiceFlags local_services() const
	{
		return local_services_;
	}
	void set_local_services(ServiceFlags flags)
	{
		local_services_ = flags;
	}
	
	const std::vector<proto_netaddr::NetAddr>& local_addrs() const
	{
		return local_addrs_;
	}
	
private:
	ServiceFlags local_services_;
	std::vector<proto_netaddr::NetAddr> local_addrs_;
	
	bool AddLocalAddr();
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
