#ifndef BTCLITE_UTILSTRENCODINGS_H
#define BTCLITE_UTILSTRENCODINGS_H

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


template<typename Iterator>
std::string HexEncode(Iterator begin, Iterator end, bool fSpaces=false)
{
    std::string rv;

    rv.reserve((end-begin)*3);
    for(Iterator it = begin; it < end; ++it)
    {
        if(fSpaces && it != begin)
            rv.push_back(' ');
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << std::hex << *it;
		rv += ss.str();
    }

    return rv;
}

#endif // BTCLITE_UTILSTRENCODINGS_H
