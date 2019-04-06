#include "net.h"

void LocalNetConfig::LookupLocalAddrs()
{
}

bool LocalNetConfig::AddLocalAddr()
{
	return true;
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


void OutboundSession::GetAddrFromSeed()
{

}

void OutboundSession::OpenConnection()
{

}
