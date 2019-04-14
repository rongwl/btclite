#include "constants.h"
#include "net.h"

void LocalNetConfig::LookupLocalAddrs()
{
}

bool LocalNetConfig::AddLocalAddr()
{
	return true;
}

bool MessageHeader::IsValid(NetworkEnv env) const
{
	if ((env == NetworkEnv::mainnet && magic_ != main_magic) ||
			(env == NetworkEnv::testnet && magic_ != testnet_magic) ||
			(env == NetworkEnv::regtest && magic_ != regtest_magic)) {
		BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::magic_(" << magic_ << ") is invalid";
		return false;
	}
	
	if (command_ != btc_message::Version::command &&
			command_ != "verack" &&
			command_ != "addr" &&
			command_ != "inv" &&
			command_ != "getdata" &&
			command_ != "merkleblock" &&
			command_ != "getblocks" &&
			command_ != "getheaders" &&
			command_ != "tx" &&
			command_ != "headers" &&
			command_ != "block" &&
			command_ != "getaddr" &&
			command_ != "mempool" &&
			command_ != "ping" &&
			command_ != "pong" &&
			command_ != "notfound" &&
			command_ != "filterload" &&
			command_ != "filteradd" &&
			command_ != "filterclear" &&
			command_ != "reject" &&
			command_ != "sendheaders" &&
			command_ != "feefilter" &&
			command_ != "sendcmpct" &&
			command_ != "cmpctblock" &&
			command_ != "getblocktxn" &&
			command_ != "blocktxn") {
		BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::command_(" << command_ << ") is invalid";
		return false;
	}
	
	if (payload_length_ > max_message_size) {
		BTCLOG(LOG_LEVEL_WARNING) << "MessageHeader::payload_length_(" << payload_length_ << ") is invalid";
		return false;
	}
	
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

