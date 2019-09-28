#include "stream.h"

#include <gtest/gtest.h>


TEST(MemIOstreamTest, OperatorIO)
{
    MemOstream ostream;
    std::string str_input = "hello world", str_output;
    std::array<int, 3> arr_input = { 1, 2, 3 };
    std::array<int, 3> arr_output;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    int num1, num2;
    
    ostream << 11 << 22 << str_input << arr_input;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> num1 >> num2 >> str_output >> arr_output;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }    
    EXPECT_EQ(num1, 11);
    EXPECT_EQ(num2, 22);
    EXPECT_EQ(str_output, str_input);
    EXPECT_EQ(arr_output, arr_input);
}
