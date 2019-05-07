#include "p2p.h"
#include "fullnode/include/config.h"

P2P::P2P()
    : network_params_(),
      local_network_config_(), 
      outbound_sessions_(),
      inbound_sessions_(),
      interrupt_(),
      thread_dns_seeds_(),
      thread_socket_handler_(),
      thread_open_connections_(),
      thread_message_handler_(),
      m_specified_outgoing_(),
      is_listen_(false),
      is_discover_(false),
      is_dnsseed_(false) {}

bool P2P::Init(BaseEnv env)
{
    network_params_.Init(env);
}

bool P2P::InitState(const Args& args)
{
    is_listen_ = args.GetBoolArg(FULLNODE_OPTION_LISTEN, true);
    is_discover_ = args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true);
    is_dnsseed_ = args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true);
    
    return true;
}

bool P2P::Start()
{
    return true;
}

bool P2P::Interrupt()
{
    return true;
}

bool P2P::Stop()
{
    return true;
}

void P2P::ThreadDnsSeeds()
{

}

void P2P::ThreadOpenConnections(const std::vector<std::string> connect)
{

}

void P2P::ThreadSocketHandler()
{

}

void P2P::ThreadMessageHandler()
{

}
