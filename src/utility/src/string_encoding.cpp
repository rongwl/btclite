#include <cstring>

#include "string_encoding.h"

void HexDecode(const std::string& in, std::vector<uint8_t> *out)
{
    auto first = std::find_if_not(in.begin(), in.end(), isspace);
    if (first == in.end())
        first = in.begin();
    if (*first == '0' && std::tolower(*(first+1)) == 'x')
        first += 2;
    auto last = std::find_if_not(first, in.end(), isxdigit);
    std::string substr(first, last);
    
    auto rit = substr.rbegin();
    while (rit != substr.rend()) {
        out->push_back(std::stoi(std::string(rit, rit+1), 0, 16));
        if (++rit != substr.rend()) {
            out->back() |= (std::stoi(std::string(rit, rit+1), 0, 16) << 4);
            rit++;
        }
    }
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

bool ParseInt32(const std::string& str, int32_t *out)
{
    if (!ParsePrechecks(str))
        return false;
    
    char *endp = nullptr;
    errno = 0; // strtol will not set errno if valid
    long int n = strtol(str.c_str(), &endp, 10);
    if(out) *out = (int32_t)n;
    // Note that strtol returns a *long int*, so even if strtol doesn't report an over/underflow
    // we still have to check that the returned value is within the range of an *int32_t*. On 64-bit
    // platforms the size of these types may be different.
    return endp && *endp == 0 && !errno &&
           n >= std::numeric_limits<int32_t>::min() &&
           n <= std::numeric_limits<int32_t>::max();
}

