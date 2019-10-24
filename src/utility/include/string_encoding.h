#ifndef BTCLITE_UTILSTRENCODINGS_H
#define BTCLITE_UTILSTRENCODINGS_H

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


namespace btclite {
namespace utility {
namespace string_encoding {

bool ParsePrechecks(const std::string& str);

/*
 * Convert string to signed 32-bit integer with strict parse error feedback.
 * @returns true if the entire string could be parsed as valid integer,
 *   false if not the entire string could be parsed or when overflow or underflow occurred.
 */
bool DecodeInt32(const std::string& str, int32_t *out);

/**
 * Convert string to signed 64-bit integer with strict parse error feedback.
 * @returns true if the entire string could be parsed as valid integer,
 *   false if not the entire string could be parsed or when overflow or underflow occurred.
 */
bool DecodeInt64(const std::string& str, int64_t *out);

/**
 * Convert decimal string to unsigned 32-bit integer with strict parse error feedback.
 * @returns true if the entire string could be parsed as valid integer,
 *   false if not the entire string could be parsed or when overflow or underflow occurred.
 */
bool DecodeUint32(const std::string& str, uint32_t *out);

/**
 * Convert decimal string to unsigned 64-bit integer with strict parse error feedback.
 * @returns true if the entire string could be parsed as valid integer,
 *   false if not the entire string could be parsed or when overflow or underflow occurred.
 */
bool DecodeUint64(const std::string& str, uint64_t *out);

/**
 * Convert string to double with strict parse error feedback.
 * @returns true if the entire string could be parsed as valid double,
 *   false if not the entire string could be parsed or when overflow or underflow occurred.
 */
bool ParseDouble(const std::string& str, double *out);

template <typename Iterator>
std::string EncodeHex(Iterator begin, Iterator end, bool fSpaces=false)
{
    std::string rv;
    static const char hexmap[16] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                     '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    rv.reserve((end-begin)*3);
    for(Iterator it = begin; it < end; ++it) {
        uint8_t val = static_cast<uint8_t>(*it);
        if(fSpaces && it != begin)
            rv.push_back(' ');
        rv.push_back(hexmap[val>>4]);
        rv.push_back(hexmap[val&15]);
    }

    return std::move(rv);
}
/*
template <typename Iterator>
void DecodeHex(const std::string& in, Iterator begin, Iterator end)
{
    auto it_in = in.begin();
    Iterator it_out = begin;
    while (it_out != end && it_in != in.end()) {
        while (isspace(*it_in))
            ++it_in;
        if (!isxdigit(*it_in))
            return;
        *it_out = std::stoi(std::string(it_in, ++it_in), 0, 16);
        if (it_in != in.end()) {
            *it_out = (*it_out << 4) | std::stoi(std::string(it_in, ++it_in), 0, 16);
        }
        ++it_out;
    }
}
*/

std::vector<uint8_t> DecodeHex(const std::string& in);

} // namespace string_encoding
} // namespace utility
} // namespace btclite


#endif // BTCLITE_UTILSTRENCODINGS_H
