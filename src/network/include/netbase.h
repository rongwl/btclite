#ifndef BTCLITE_NETBASE_H
#define BTCLITE_NETBASE_H

#include "network_address.h"


namespace btclite {
namespace network {

bool LookupHost(const char *psz_name, NetAddr *out, bool allow_lookup);
bool LookupHost(const char *psz_name, std::vector<NetAddr> *out, unsigned int max_solutions, bool allow_lookup);
bool LookupSubNet(const char* pszName, SubNet *out);
bool LookupIntern(const char *psz_name, std::vector<NetAddr> *out, unsigned int max_solutions, bool allow_lookup);

} // namespace network
} // namespace btclite


#endif // BTCLITE_NETBASE_H
