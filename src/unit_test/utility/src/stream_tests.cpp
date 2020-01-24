#include "stream.h"

#include <gtest/gtest.h>


namespace btclite {
namespace unit_test {

TEST(MemIOstreamTest, OperatorIO)
{
    util::MemoryStream ms;
    std::string str_input = "hello world", str_output;
    std::array<int, 3> arr_input = { 1, 2, 3 };
    std::array<int, 3> arr_output;
    std::vector<uint8_t> vec;
    int num1, num2;
    
    ms << 11 << 22 << str_input << arr_input;
    EXPECT_NO_THROW(ms >> num1 >> num2 >> str_output >> arr_output);
    EXPECT_EQ(num1, 11);
    EXPECT_EQ(num2, 22);
    EXPECT_EQ(str_output, str_input);
    EXPECT_EQ(arr_output, arr_input);
}

} // namespace unit_test
} // namespace btclit
