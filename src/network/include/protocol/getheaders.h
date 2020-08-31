#ifndef BTCLITE_PROTOCOL_GETHEADERS_H
#define BTCLITE_PROTOCOL_GETHEADERS_H


#include "protocol/getblocks.h"


namespace btclite {
namespace network {
namespace protocol {

class GetHeaders : public GetBlocks {
public:
    bool RecvHandler(std::shared_ptr<Node> src_node, const Params& params) const;    
    std::string Command() const;
};

} // namespace protocol
} // namespace network
} // namespace btclite

#endif // BTCLITE_PROTOCOL_GETHEADERS_H
