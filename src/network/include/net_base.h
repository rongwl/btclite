#ifndef BTCLITE_NETBASE_H
#define BTCLITE_NETBASE_H

#include "network_address.h"


namespace btclite {
namespace network {

using NodeId = int64_t;

bool LookupHost(const char *psz_name, NetAddr *out, 
                bool allow_lookup, uint16_t port);
bool LookupHost(const char *psz_name, std::vector<NetAddr> *out, 
                unsigned int max_solutions, bool allow_lookup,
                uint16_t port);
bool LookupSubNet(const char* pszName, uint16_t port, SubNet *out);
bool LookupIntern(const char *psz_name, std::vector<NetAddr> *out, 
                  unsigned int max_solutions, bool allow_lookup,
                  uint16_t port);

} // namespace network
} // namespace btclite


#endif // BTCLITE_NETBASE_H
