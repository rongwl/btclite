#ifndef BTCLITE_UTILSTRENCODINGS_H
#define BTCLITE_UTILSTRENCODINGS_H

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

signed char HexDigit(char c);

template<typename T>
std::string HexStr(const T itbegin, const T itend, bool fSpaces=false)
{
    std::string rv;

    rv.reserve((itend-itbegin)*3);
    for(T it = itbegin; it < itend; ++it)
    {
        if(fSpaces && it != itbegin)
            rv.push_back(' ');
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << std::hex << *it;
		rv += ss.str();
    }

    return rv;
}

#endif // BTCLITE_UTILSTRENCODINGS_H
