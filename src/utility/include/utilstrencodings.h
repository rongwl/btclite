#ifndef BTCLITE_UTILSTRENCODINGS_H
#define BTCLITE_UTILSTRENCODINGS_H

#include <cstddef>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>


template <typename Iterator>
std::string HexEncode(Iterator begin, Iterator end, bool fSpaces=false)
{
    std::string rv;

    rv.reserve((end-begin)*3);
    for(Iterator it = begin; it < end; ++it)
    {
        if(fSpaces && it != begin)
            rv.push_back(' ');
		std::stringstream ss;
		ss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(*it);
		rv += ss.str();
    }

    return rv;
}

template <typename Iterator>
void HexDecode(const std::string& in, Iterator begin, Iterator end)
{
	auto first = std::find_if_not(in.begin(), in.end(), isspace);
	if (first == in.end())
		first = in.begin();
	if (*first == '0' && std::tolower(*(first+1)) == 'x')
		first += 2;
	auto last = std::find_if_not(first, in.end(), isxdigit);
	std::string substr(first, last);
	
	for (auto it = begin, rit = substr.rbegin(); it != end && rit < substr.rend(); ++it, rit += 2) {
		std::string low(rit, rit+1), hi(rit+1, rit+2);
		*it = std::stoi(hi) << 4 | std::stoi(low);
	}
}

void HexDecode(const std::string& in, std::vector<uint8_t> *);

#endif // BTCLITE_UTILSTRENCODINGS_H
