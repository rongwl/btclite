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
	
	auto rit = substr.rbegin();
	while (rit != substr.rend()) {
		out->push_back(std::stoi(std::string(rit, rit+1), 0, 16));
		if (++rit != substr.rend()) {
			out->back() |= (std::stoi(std::string(rit, rit+1), 0, 16) << 4);
			rit++;
		}
	}
}

