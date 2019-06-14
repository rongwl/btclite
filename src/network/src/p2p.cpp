#include "p2p.h"
#include "fullnode/include/config.h"


Nodes P2P::nodes_;
std::map<Node::NodeId, NodeState> P2P::map_node_state_;

P2P::P2P(const ExecutorConfig& config)
    : executor_config_(config),
      network_params_(config.env()),
      network_args_(config.args()),
      ban_db_(config.data_files()) {}

bool P2P::Init()
{    
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
    while (!interrupt_) {

    }
}

void P2P::ThreadMessageHandler()
{

}
