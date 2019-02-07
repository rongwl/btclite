#include "utilstrencodings.h"

void HexDecode(const std::string& in, std::vector<uint8_t> *out)
{
	auto first = std::find_if_not(in.begin(), in.end(), isspace);
	if (first == in.end())
		first = in.begin();
	if (*first == '0' && std::tolower(*(first+1)) == 'x')
		first += 2;
	auto last = std::find_if_not(first, in.end(), isxdigit);
	std::string substr(first, last);
	
	for (auto rit = substr.rbegin(); rit < substr.rend(); rit += 2) {
		std::string low(rit, rit+1), hi(rit+1, rit+2);
		out->push_back(std::stoi(hi) << 4 | std::stoi(low));
	}
}

