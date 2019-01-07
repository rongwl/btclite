#include <algorithm>
#include <iterator>

#include "uint256.h"
#include "utilstrencodings.h"

template <unsigned int BITS>
std::string BaseBlob<BITS>::GetHex() const
{
	return HexStr(std::reverse_iterator<const std::byte*>(data + sizeof(data)), std::reverse_iterator<const std::byte*>(data));
}

template <unsigned int BITS>
void BaseBlob<BITS>::SetHex(const std::string& psz)
{
	std::memset(data, 0, sizeof(data));

	auto first = std::find_if_not(psz.begin(), psz.end(), isspace);
	if (first == psz.end())
		first = psz.begin();
	if (*first == '0' && std::tolower(*(first+1)) == 'x')
		first += 2;
	auto last = std::find_if_not(first, psz.end(), isxdigit);
	std::string substr(first, last);
	
	std::byte* p = data;
	for (auto rit = substr.rbegin(); rit < substr.rend(); rit += 2) {
		std::string low(rit, rit+1), hi(rit+1, rit+2);
		*p = std::stoi(hi) << 4 | std::stoi(low);
		p++;
	}
}

template <unsigned int BITS>
std::string BaseBlob<BITS>::ToString() const
{
    return (GetHex());
}
