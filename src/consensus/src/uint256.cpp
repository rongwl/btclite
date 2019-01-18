#include <algorithm>
#include <iterator>

#include "uint256.h"
#include "utilstrencodings.h"

template <unsigned int BITS>
std::string BaseBlob<BITS>::GetHex() const
{
	return HexStr(this->rbegin(), this->rend());
}

template <unsigned int BITS>
void BaseBlob<BITS>::SetHex(const std::string& psz)
{
	SetNull();

	auto first = std::find_if_not(psz.begin(), psz.end(), isspace);
	if (first == psz.end())
		first = psz.begin();
	if (*first == '0' && std::tolower(*(first+1)) == 'x')
		first += 2;
	auto last = std::find_if_not(first, psz.end(), isxdigit);
	std::string substr(first, last);
	
	for (auto it = this->begin(), rit = substr.rbegin(); rit < substr.rend(); ++it, rit += 2) {
		std::string low(rit, rit+1), hi(rit+1, rit+2);
		*it = std::stoi(hi) << 4 | std::stoi(low);
	}
}
