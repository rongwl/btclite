#include "script.h"


namespace btclite {
namespace consensus {

ScriptInt::ScriptInt(const int64_t& n)
{
    set_value(n);
}

ScriptInt::ScriptInt(const std::vector<uint8_t>& v, bool minimal)
{
    if (v.size() == 0)
        return;
    if (v.size() > max_size)
        throw std::runtime_error("script number overflow");
    
    if (minimal) {
        // Check that the number is encoded with the minimum possible
        // number of bytes.
        //
        // If the most-significant-byte - excluding the sign bit - is zero
        // then we're not minimal. Note how this test also rejects the
        // negative-zero encoding, 0x80.
        if ((v.back() & 0x7f) == 0) {
            // One exception: if there's more than one byte and the most
            // significant bit of the second-most-significant-byte is set
            // it would conflict with the sign bit. An example of this case
            // is +-255, which encode to 0xff00 and 0xff80 respectively.
            // (big-endian).
            if (v.size() < 2 || (v[v.size()-2] & 0x80) == 0)
                throw std::runtime_error("non-minimally encoded script number");
        }
    }
    
    int64_t value = 0;
    for (unsigned int i = 0; i < v.size(); i++) 
        value |= (v[i] << 8*i);
    
    // If the input vector's most significant byte is 0x80, remove it from
    // the result's msb and return a negative.
    if (v.back() & 0x80) {
        value &= ~(0x80ULL << (8 * (v.size() - 1)));
        value = -value;
    }
    set_value(value);
}

int ScriptInt::IntValue() const
{
    if (value() > std::numeric_limits<int32_t>::max())
        return std::numeric_limits<int32_t>::max();
    if (value() < std::numeric_limits<int32_t>::min())
        return std::numeric_limits<int32_t>::min();
    return value();
}

std::vector<uint8_t> ScriptInt::BytesValue() const
{
    return BytesEncoding(value());
}

std::vector<uint8_t> ScriptInt::BytesEncoding(const uint64_t& value)
{
    if (value == 0)
        return std::vector<uint8_t>();
    
    std::vector<uint8_t> result;
    bool neg = (value < 0);
    int64_t abs = neg ? -value : value;
    
    while (abs) {
        result.push_back(abs & 0xff);
        abs >>= 8;
    }
    
//    - If the most significant byte is >= 0x80 and the value is positive, push a
//    new zero-byte to make the significant byte < 0x80 again.

//    - If the most significant byte is >= 0x80 and the value is negative, push a
//    new 0x80 byte that will be popped off when converting to an integral.

//    - If the most significant byte is < 0x80 and the value is negative, add
//    0x80 to it, since it will be subtracted and interpreted as a negative when
//    converting to an integral.
    if (result.back() & 0x80)
        result.push_back(neg ? 0x80 : 0);
    else if (neg)
        result.push_back(0x80);
    
    return result;
}

Script::Script(std::vector<uint8_t>&& v) noexcept
    : data_(std::move(v)) 
{
}

Script::Script(const std::vector<uint8_t>& v)
    : data_(v) 
{
}

Script::Script(Script&& s) noexcept
    : data_(std::move(s.data_)) 
{
}

Script::Script(const Script& s)
    : data_(s.data_)
{
}

void Script::Push(const uint64_t& b)
{
    if (b == -1 || (b >= 1 && b <= 16))
        data_.push_back(b + static_cast<uint8_t>(Opcode::OP_1) - 1);
    else if (b == 0)
        data_.push_back(static_cast<uint8_t>(Opcode::OP_0));
    else
        this->Push(ScriptInt::BytesEncoding(b));
}

void Script::Push(const Opcode& code)
{
    data_.push_back(static_cast<uint8_t>(code));
}

void Script::Push(const ScriptInt& sint)
{
    this->Push(sint.BytesValue());
}    

void Script::Push(const std::vector<uint8_t>& b)
{
    if (b.size() < static_cast<uint8_t>(Opcode::OP_PUSHDATA1))
        data_.push_back(static_cast<uint8_t>(b.size()));
    else if (b.size() <= 0xff) {
        data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA1));
        data_.push_back(static_cast<uint8_t>(b.size()));
    }
    else if (b.size() <= 0xffff) {
        data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA2));
        uint8_t size[2];
        util::ToLittleEndian(static_cast<uint16_t>(b.size()), size);
        data_.push_back(size[0]);
        data_.push_back(size[1]);
    }
    else {
        data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA4));
        uint8_t size[4];
        util::ToLittleEndian(static_cast<uint32_t>(b.size()), size);
        for (auto elem : size) {
            data_.push_back(elem);
        }
    }
    data_.insert(data_.end(), b.begin(), b.end());
}

void Script::clear()
{
    data_.clear();
}

bool Script::Pop(std::vector<uint8_t>::const_iterator& pc, Opcode *out) const
{
    ASSERT_NULL(out);
    if (pc >= data_.end())
        return false;
    
    *out = static_cast<Opcode>(*pc);
    pc++;    
    
    return true;
}

bool Script::Pop(std::vector<uint8_t>::const_iterator& pc, const Opcode& in, std::vector<uint8_t> *out) const
{
    ASSERT_NULL(out);
    if (pc >= data_.end())
        return false;
    if (in == Opcode::OP_0 || in > Opcode::OP_PUSHDATA4)
        return false;
    
    uint32_t size;
    if (in < Opcode::OP_PUSHDATA1) {
        size = static_cast<uint8_t>(in);
    }
    else if (in == Opcode::OP_PUSHDATA1) {
        size = *pc++;
    }
    else if (in == Opcode::OP_PUSHDATA2) {
        if (pc + 2 > data_.end())
            return false;
        size = util::FromLittleEndian<uint16_t>(&(*pc));
        pc += 2;
    }
    else if (in == Opcode::OP_PUSHDATA4) {
        if (pc + 4 > data_.end())
            return false;
        size = util::FromLittleEndian<uint32_t>(&(*pc));
        pc += 4;
    }
    
    if (pc + size > data_.end())
        return false;
    out->assign(pc, pc+size);
    
    return true;
}

std::vector<uint8_t>::const_iterator Script::begin() const
{
    return data_.begin();
}

std::vector<uint8_t>::const_iterator Script::end() const
{
    return data_.end();
}

std::vector<uint8_t>::const_reverse_iterator Script::rbegin() const
{
    return data_.rbegin();
}

std::vector<uint8_t>::const_reverse_iterator Script::rend() const
{
    return data_.rend();
}

bool Script::operator==(const Script& b) const
{
    return data_ == b.data_;
}

bool Script::operator!=(const Script& b) const
{
    return !(*this == b);
}

Script& Script::operator=(const Script& b)
{
    data_ = b.data_;
    return *this;
}

Script& Script::operator=(Script&& b) noexcept
{
    if (this != &b) {
        data_ = std::move(b.data_);
    }
    return *this;
}

size_t Script::SerializedSize() const
{
    size_t result = data_.size();
    result += util::VarIntSize(result);
    return result;
}

} // namespace consensus
} // namespace btclite
