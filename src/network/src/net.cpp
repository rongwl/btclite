#include "net.h"

void LocalNetConfig::LookupLocalAddrs()
{
}

bool LocalNetConfig::AddLocalAddr()
{
	return true;
}

void Message::DataFactory(const std::string& command)
{
	if (command == btc_message::Version::command)
		data_ = std::make_shared<btc_message::Version>();
	else
		data_.reset();
}

bool Message::RecvMsgHandle()
{
	if (!data_.use_count())
		return false;
	
	return data_.get()->RecvMsgHandle();
}

Socket BaseSocket::Create()
{
	return 0;
}

bool BaseSocket::Close()
{
	return true;
}

bool Connector::Connect()
{
	return true;
}

bool Acceptor::Bind()
{
	return true;
}

Socket Acceptor::Accept()
{
	return 0;
}

void BaseNode::Connect()
{

}

void BaseNode::Disconnect()
{

}

size_t BaseNode::Receive()
{
	return 0;
}

size_t BaseNode::Send()
{
	return 0;
}

void OutboundSession::GetAddrFromSeed()
{

}

void OutboundSession::OpenConnection()
{

}

