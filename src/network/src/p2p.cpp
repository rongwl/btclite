#include "p2p.h"

bool P2P::Init(NetworkEnv env)
{
    network_params_.Init(env);
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
