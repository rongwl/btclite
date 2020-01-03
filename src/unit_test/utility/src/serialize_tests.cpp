#include "stream.h"

#include <gtest/gtest.h>


TEST(SerializerTest, SerializeArithmetic)
{
    double dinput = 1.23456789, doutput;
    float finput = 1.234, foutput;
    uint64_t iinput = 0x1122334455667788, ioutput;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << dinput << finput << iinput;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    EXPECT_NO_THROW(istream >> doutput >> foutput >> ioutput);
    EXPECT_EQ(dinput, doutput);
    EXPECT_EQ(finput, foutput);
    EXPECT_EQ(iinput, ioutput);
}

TEST(SerializerTest, SerializeEnum)
{
    enum class Foo { kBar1, kBar2, kBar3 };
    Foo out1, out2;
    MemOstream ostream;
    std::vector<uint8_t> vec;
    ByteSource<std::vector<uint8_t> > byte_source(vec);
    MemIstream<ByteSource<std::vector<uint8_t> > > istream(byte_source);
    
    ostream << Foo::kBar1 << Foo::kBar2;
    vec.assign(ostream.vec().begin(), ostream.vec().end());
    EXPECT_NO_THROW(istream >> out1 >> out2);
    EXPECT_EQ(out1, Foo::kBar1);
    EXPECT_EQ(out2, Foo::kBar2);
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
    EXPECT_NO_THROW(istream >> output);
    EXPECT_EQ(input, output);
    
    input.clear();
    ostream << input;
    EXPECT_NO_THROW(istream >> output);
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
    EXPECT_NO_THROW(istream >> ioutput >> foutput >> doutput);
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
    EXPECT_NO_THROW(istream >> ioutput >> foutput >> doutput);
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
    EXPECT_NO_THROW(istream >> output);
    EXPECT_EQ(input, output);
}


