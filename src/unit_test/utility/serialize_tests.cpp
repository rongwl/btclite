#include "stream.h"

#include <gtest/gtest.h>


TEST(SerializerTest, SerializeArithmetic)
{
    double dinput = 1.23456789, doutput;
    float finput = 1.234, foutput;
    int iinput = 123, ioutput;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << dinput << finput << iinput;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> doutput >> foutput >> ioutput;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }
    EXPECT_EQ(dinput, doutput);
    EXPECT_EQ(finput, foutput);
    EXPECT_EQ(iinput, ioutput);
}

TEST(SerializerTest, SerializeString)
{
    std::string input = "foo bar", output;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << input;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> output;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }
    EXPECT_EQ(input, output);
}

TEST(SerializerTest, SerializeArithmeticArray)
{
    std::array<int, 3> iinput = { 1, 2, 3 }, ioutput;
    std::array<float, 3> finput = { 1.1, 2.2, 3.3}, foutput;
    std::array<double, 3> dinput = { 1.11111111, 2.22222222, 3.33333333}, doutput;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << iinput << finput << dinput;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> ioutput >> foutput >> doutput;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }
    EXPECT_EQ(iinput, ioutput);
    EXPECT_EQ(dinput, doutput);
    EXPECT_EQ(finput, foutput);
}

TEST(SerializerTest, SerializeArithmeticVector)
{
    std::vector<int> iinput = { 1, 2, 3 }, ioutput;
    std::vector<float> finput = { 1.1, 2.2, 3.3}, foutput;
    std::vector<double> dinput = { 1.11111111, 2.22222222, 3.33333333}, doutput;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << iinput << finput << dinput;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> ioutput >> foutput >> doutput;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }
    EXPECT_EQ(iinput, ioutput);
    EXPECT_EQ(dinput, doutput);
    EXPECT_EQ(finput, foutput);
}

TEST(SerializerTest, SerializeClassVector)
{
    std::vector<std::string> input = { "foo", "bar" }, output;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << input;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    try {
        istream >> output;
    }
    catch (const std::exception& e) {
        FAIL() << "Exception:" << e.what();
    }
    EXPECT_EQ(input, output);
}


