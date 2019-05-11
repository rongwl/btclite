#include "p2p.h"
#include "fullnode/include/config.h"

P2P::P2P()
    : network_params_(),
      network_args_(),
      local_network_config_(), 
      outbound_sessions_(),
      inbound_sessions_(),
      interrupt_(),
      thread_dns_seeds_(),
      thread_socket_handler_(),
      thread_open_connections_(),
      thread_message_handler_() {}

bool P2P::Init(BaseEnv env)
{
    network_params_.Init(env);
}

bool P2P::InitArgs(const Args& args)
{
    network_args_.is_listen_ = args.GetBoolArg(FULLNODE_OPTION_LISTEN, true);
    network_args_.is_discover_ = args.GetBoolArg(FULLNODE_OPTION_DISCOVER, true);
    network_args_.is_dnsseed_ = args.GetBoolArg(FULLNODE_OPTION_DNSSEED, true);
    if (args.IsArgSet(FULLNODE_OPTION_CONNECT))
        network_args_.specified_outgoing_ = args.GetArgs(FULLNODE_OPTION_CONNECT);
    
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
