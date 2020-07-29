#include <arithmetic.h>


namespace btclite {
namespace util {

Uint256::Uint256(const std::string& s)
{
    std::vector<uint8_t> vec;
    auto start_it = s.begin();
    auto end_it = s.end()-1;
    
    // skip leading space
    while (std::isspace(*start_it)) {
        start_it++;
    }
    
    // skip 0x
    if (s.end() - start_it > 2 && 
            *start_it == '0' && std::tolower(*(start_it+1)) == 'x') {
        start_it += 2;
    }
    
    // skip ending space
    while (std::isspace(*end_it)) {
        end_it--;
    }
    
    assert(std::find_if(start_it, end_it+1, 
           [](char c){ return std::isxdigit(c) == 0; }) == end_it+1);
    std::string sub_str = s.substr(start_it - s.begin(), end_it - start_it + 1);    
    vec = DecodeHex((sub_str.size() % 2 == 0) ? sub_str : "0" + sub_str);
    
    if (vec.size() > this->Size()) {
        std::copy(vec.rbegin(), vec.rbegin() + this->Size(), this->begin());
    }
    else {
        std::copy(vec.rbegin(), vec.rend(), this->begin());
    }
}

} // namespace util
} // namespace btclite
