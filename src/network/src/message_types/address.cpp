#include "message_types/address.h"


namespace btclite {
namespace network {
namespace message_types {

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

} // namespace message_types
} // namespace network
} // namespace btclite
