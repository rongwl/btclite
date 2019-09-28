#ifndef BTCLITE_NETBASE_H
#define BTCLITE_NETBASE_H

#include "network_address.h"


bool LookupHost(const char *psz_name, btclite::network::NetAddr *out, bool allow_lookup);
bool LookupHost(const char *psz_name, std::vector<btclite::network::NetAddr> *out, unsigned int max_solutions, bool allow_lookup);
bool LookupSubNet(const char* pszName, SubNet *out);
bool LookupIntern(const char *psz_name, std::vector<btclite::network::NetAddr> *out, unsigned int max_solutions, bool allow_lookup);


#endif // BTCLITE_NETBASE_H
