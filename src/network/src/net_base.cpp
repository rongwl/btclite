#include "net_base.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "network/include/params.h"
#include "string_encoding.h"


namespace btclite {
namespace network {

bool LookupHost(const char *psz_name, NetAddr *out, 
                bool allow_lookup, uint16_t default_port)
{
    std::vector<NetAddr> vip;
    LookupHost(psz_name, &vip, 1, allow_lookup, default_port);
    if(vip.empty())
        return false;
    *out = vip.front();
    
    return true;
}

bool LookupHost(const char *psz_name, std::vector<NetAddr> *out,
                                unsigned int max_solutions, bool allow_lookup,
                                uint16_t default_port)
{
    std::string str_host(psz_name);
    if (str_host.empty())
        return false;
    
    if (str_host[0] == '[' && str_host[str_host.size()-1] == ']')
    {
        str_host = str_host.substr(1, str_host.size() - 2);
    }

    return LookupIntern(str_host.c_str(), out, max_solutions, 
                        allow_lookup, default_port);
}

bool LookupSubNet(const char* psz_name, uint16_t default_port, SubNet *out)
{
    std::string str_subnet(psz_name);
    size_t slash = str_subnet.find_last_of('/');
    std::vector<NetAddr> vip;

    std::string str_addr = str_subnet.substr(0, slash);
    if (LookupHost(str_addr.c_str(), &vip, 1, false, default_port))
    {
        NetAddr network = vip[0];
        if (slash != str_subnet.npos)
        {
            std::string str_netmask = str_subnet.substr(slash + 1);
            int32_t n;
            // IPv4 addresses start at offset 12, and first 12 bytes must match, so just offset n
            // If valid number, assume /24 syntax
            if (util::DecodeInt32(str_netmask, &n)) {                
                *out = SubNet(network, n);
                return out->IsValid();
            }
            else // If not a valid number, try full netmask syntax
            {
                // Never allow lookup for netmask
                if (LookupHost(str_netmask.c_str(), &vip, 1,
                               false, default_port)) {
                    *out = SubNet(network, vip[0]);
                    return out->IsValid();
                }
            }
        }
        else
        {
            *out = SubNet(network);
            return out->IsValid();
        }
    }
    
    return false;
}

bool LookupIntern(const char *psz_name, std::vector<NetAddr> *out,
                                  unsigned int max_solutions, bool allow_lookup,
                                  uint16_t default_port)
{
    struct addrinfo aiHint;
    memset(&aiHint, 0, sizeof(struct addrinfo));

    out->clear();
    
    aiHint.ai_socktype = SOCK_STREAM;
    aiHint.ai_protocol = IPPROTO_TCP;
    aiHint.ai_family = AF_UNSPEC;
    aiHint.ai_flags = allow_lookup ? AI_ADDRCONFIG : AI_NUMERICHOST;

    struct addrinfo *aiRes = nullptr;
    if (getaddrinfo(psz_name, nullptr, &aiHint, &aiRes))
        return false;

    struct addrinfo *aiTrav = aiRes;
    while (aiTrav != nullptr && (max_solutions == 0 || out->size() < max_solutions))
    {
        NetAddr resolved;
        if (aiTrav->ai_family == AF_INET)
        {
            assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in));
            resolved.SetIpv4(((struct sockaddr_in*)(aiTrav->ai_addr))->sin_addr.s_addr);
        }
        else if (aiTrav->ai_family == AF_INET6)
        {
            assert(aiTrav->ai_addrlen >= sizeof(sockaddr_in6));
            resolved.SetIpv6(((struct sockaddr_in6*)aiTrav->ai_addr)->sin6_addr.s6_addr);
            resolved.set_scope_id(((struct sockaddr_in6*)aiTrav->ai_addr)->sin6_scope_id);
        }
        
        /* Never allow resolving to an internal address. Consider any such result invalid */
        if (!resolved.IsInternal()) {
            resolved.set_port(default_port);
            out->push_back(resolved);
        }

        aiTrav = aiTrav->ai_next;
    }

    freeaddrinfo(aiRes);

    return (out->size() > 0);
}

} // namespace network
} // namespace btclite
