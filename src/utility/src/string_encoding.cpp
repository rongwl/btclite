#include "string_encoding.h"

#include <cstring>


namespace btclite {
namespace util {

std::vector<uint8_t> DecodeHex(const std::string& in)
{
    std::vector<uint8_t> result;
    
    auto it = in.begin();
    while (it != in.end()) {
        while (isspace(*it))
            ++it;
        if (!isxdigit(*it))
            return std::move(result);
        result.push_back(std::stoi(std::string(it, it+1), 0, 16));
        if (++it != in.end()) {
            result.back() = (result.back() <<= 4) | std::stoi(std::string(it, it+1), 0, 16);
            ++it;
        }
    }
    
    return std::move(result);
}

bool ParsePrechecks(const std::string& str)
{
    if (str.empty()) // No empty string allowed
        return false;
    if (str.size() >= 1 && (isspace(str[0]) || isspace(str[str.size()-1]))) // No padding allowed
        return false;
    if (str.size() != std::strlen(str.c_str())) // No embedded NUL characters allowed
        return false;
    
    return true;
}

bool DecodeInt32(const std::string& str, int32_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    
    char *endp = nullptr;
    errno = 0; // strtol will not set errno if valid
    long int n = strtol(str.c_str(), &endp, 10);
    if (out) 
        *out = (int32_t)n;
    // Note that strtol returns a *long int*, so even if strtol doesn't report an over/underflow
    // we still have to check that the returned value is within the range of an *int32_t*. On 64-bit
    // platforms the size of these types may be different.
    return endp && *endp == 0 && !errno &&
           n >= std::numeric_limits<int32_t>::min() &&
           n <= std::numeric_limits<int32_t>::max();
}

bool DecodeInt64(const std::string& str, int64_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    char *endp = nullptr;
    errno = 0; // strtoll will not set errno if valid
    long long int n = strtoll(str.c_str(), &endp, 10);
    if(out)
        *out = (int64_t)n;
    // Note that strtoll returns a *long long int*, so even if strtol doesn't report an over/underflow
    // we still have to check that the returned value is within the range of an *int64_t*.
    return endp && *endp == 0 && !errno &&
           n >= std::numeric_limits<int64_t>::min() &&
           n <= std::numeric_limits<int64_t>::max();
}

bool DecodeUint32(const std::string& str, uint32_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    if (str.size() >= 1 && str[0] == '-') // Reject negative values, unfortunately strtoul accepts these by default if they fit in the range
        return false;
    char *endp = nullptr;
    errno = 0; // strtoul will not set errno if valid
    unsigned long int n = strtoul(str.c_str(), &endp, 10);
    if(out) 
        *out = (uint32_t)n;
    // Note that strtoul returns a *unsigned long int*, so even if it doesn't report an over/underflow
    // we still have to check that the returned value is within the range of an *uint32_t*. On 64-bit
    // platforms the size of these types may be different.
    return endp && *endp == 0 && !errno &&
           n <= std::numeric_limits<uint32_t>::max();
}

bool DecodeUint64(const std::string& str, uint64_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    if (str.size() >= 1 && str[0] == '-') // Reject negative values, unfortunately strtoull accepts these by default if they fit in the range
        return false;
    char *endp = nullptr;
    errno = 0; // strtoull will not set errno if valid
    unsigned long long int n = strtoull(str.c_str(), &endp, 10);
    if(out)
        *out = (uint64_t)n;
    // Note that strtoull returns a *unsigned long long int*, so even if it doesn't report an over/underflow
    // we still have to check that the returned value is within the range of an *uint64_t*.
    return endp && *endp == 0 && !errno &&
           n <= std::numeric_limits<uint64_t>::max();
}


bool DecodeDouble(const std::string& str, double *out)
{
    if (!ParsePrechecks(str))
        return false;
    if (str.size() >= 2 && str[0] == '0' && str[1] == 'x') // No hexadecimal floats allowed
        return false;
    std::istringstream text(str);
    text.imbue(std::locale::classic());
    double result;
    text >> result;
    if(out)
        *out = result;
    return text.eof() && !text.fail();
}

} // namespace util
} // namespace btclite
