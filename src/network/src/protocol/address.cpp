#include "protocol/address.h"


namespace btclite {
namespace network {
namespace protocol {

bool NetAddr::IsValid() const
{
    return (timestamp != 0) ||
           (services != 0) ||
           (ip != kNullIp) ||
           (port != 0);
}

void NetAddr::Clear()
{
    timestamp = 0;
    services = 0;
    ip = kNullIp;
    port = 0;
}

bool NetAddr::operator==(const NetAddr& b) const
{
    return (timestamp == b.timestamp) &&
           (services == b.services) &&
           (ip == b.ip) &&
           (port == b.port);
}

bool NetAddr::operator!=(const NetAddr& b) const
{
    return !(*this == b);
}

} // namespace protocol
} // namespace network
} // namespace btclite
