#ifndef BTCLITE_UTILSTRENCODINGS_H
#define BTCLITE_UTILSTRENCODINGS_H

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


template <typename Iterator>
std::string HexEncode(Iterator rbegin, Iterator rend, bool fSpaces=false)
{
    std::string rv;

    rv.reserve((rend-rbegin)*3);
    for(Iterator it = rbegin; it < rend; ++it)
    {
        if(fSpaces && it != rbegin)
            rv.push_back(' ');
        std::stringstream ss;
        ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*it);
        rv += ss.str();
    }

    return rv;
}

template <typename Iterator>
void HexDecode(const std::string& in, Iterator begin, Iterator end) // little endian storage
{
    auto first = std::find_if_not(in.begin(), in.end(), isspace);
    if (first == in.end())
        first = in.begin();
    if (*first == '0' && std::tolower(*(first+1)) == 'x')
        first += 2;
    auto last = std::find_if_not(first, in.end(), isxdigit);
    std::string substr(first, last);
    
    auto rit = substr.rbegin();
    Iterator it = begin;
    while (it != end && rit != substr.rend()) {
        *it = std::stoi(std::string(rit, rit+1), 0, 16);
        if (++rit != substr.rend()) {
            *it |= (std::stoi(std::string(rit, rit+1), 0, 16) << 4);
            rit++;
        }
        it++;
    }
}

void HexDecode(const std::string&, std::vector<uint8_t>*);

template <typename Iterator>
void ReverseEndian(Iterator begin, Iterator end)
{    
    end--;
    while (begin < end) {
        std::swap(*begin, *end);
        begin++;
        end--;
    }
}

#endif // BTCLITE_UTILSTRENCODINGS_H
