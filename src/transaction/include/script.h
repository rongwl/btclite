#ifndef BTCLITE_SCRIPT_H
#define BTCLITE_SCRIPT_H

#include <stdexcept>
#include <vector>
#include <utility>

#include "arithmetic.h"
#include "constants.h"
#include "serialize.h"

enum class Opcode : uint8_t {
	// push value
    OP_0 = 0x00,
    OP_FALSE = OP_0,
    OP_PUSHDATA1 = 0x4c,
    OP_PUSHDATA2 = 0x4d,
    OP_PUSHDATA4 = 0x4e,
    OP_1NEGATE = 0x4f,
    OP_RESERVED = 0x50,
    OP_1 = 0x51,
    OP_TRUE=OP_1,
    OP_2 = 0x52,
    OP_3 = 0x53,
    OP_4 = 0x54,
    OP_5 = 0x55,
    OP_6 = 0x56,
    OP_7 = 0x57,
    OP_8 = 0x58,
    OP_9 = 0x59,
    OP_10 = 0x5a,
    OP_11 = 0x5b,
    OP_12 = 0x5c,
    OP_13 = 0x5d,
    OP_14 = 0x5e,
    OP_15 = 0x5f,
    OP_16 = 0x60,

    // control
    OP_NOP = 0x61,
    OP_VER = 0x62,
    OP_IF = 0x63,
    OP_NOTIF = 0x64,
    OP_VERIF = 0x65,
    OP_VERNOTIF = 0x66,
    OP_ELSE = 0x67,
    OP_ENDIF = 0x68,
    OP_VERIFY = 0x69,
    OP_RETURN = 0x6a,

    // stack ops
    OP_TOALTSTACK = 0x6b,
    OP_FROMALTSTACK = 0x6c,
    OP_2DROP = 0x6d,
    OP_2DUP = 0x6e,
    OP_3DUP = 0x6f,
    OP_2OVER = 0x70,
    OP_2ROT = 0x71,
    OP_2SWAP = 0x72,
    OP_IFDUP = 0x73,
    OP_DEPTH = 0x74,
    OP_DROP = 0x75,
    OP_DUP = 0x76,
    OP_NIP = 0x77,
    OP_OVER = 0x78,
    OP_PICK = 0x79,
    OP_ROLL = 0x7a,
    OP_ROT = 0x7b,
    OP_SWAP = 0x7c,
    OP_TUCK = 0x7d,

    // splice ops
    OP_CAT = 0x7e,
    OP_SUBSTR = 0x7f,
    OP_LEFT = 0x80,
    OP_RIGHT = 0x81,
    OP_SIZE = 0x82,

    // bit logic
    OP_INVERT = 0x83,
    OP_AND = 0x84,
    OP_OR = 0x85,
    OP_XOR = 0x86,
    OP_EQUAL = 0x87,
    OP_EQUALVERIFY = 0x88,
    OP_RESERVED1 = 0x89,
    OP_RESERVED2 = 0x8a,

    // numeric
    OP_1ADD = 0x8b,
    OP_1SUB = 0x8c,
    OP_2MUL = 0x8d,
    OP_2DIV = 0x8e,
    OP_NEGATE = 0x8f,
    OP_ABS = 0x90,
    OP_NOT = 0x91,
    OP_0NOTEQUAL = 0x92,

    OP_ADD = 0x93,
    OP_SUB = 0x94,
    OP_MUL = 0x95,
    OP_DIV = 0x96,
    OP_MOD = 0x97,
    OP_LSHIFT = 0x98,
    OP_RSHIFT = 0x99,

    OP_BOOLAND = 0x9a,
    OP_BOOLOR = 0x9b,
    OP_NUMEQUAL = 0x9c,
    OP_NUMEQUALVERIFY = 0x9d,
    OP_NUMNOTEQUAL = 0x9e,
    OP_LESSTHAN = 0x9f,
    OP_GREATERTHAN = 0xa0,
    OP_LESSTHANOREQUAL = 0xa1,
    OP_GREATERTHANOREQUAL = 0xa2,
    OP_MIN = 0xa3,
    OP_MAX = 0xa4,

    OP_WITHIN = 0xa5,

    // crypto
    OP_RIPEMD160 = 0xa6,
    OP_SHA1 = 0xa7,
    OP_SHA256 = 0xa8,
    OP_HASH160 = 0xa9,
    OP_HASH256 = 0xaa,
    OP_CODESEPARATOR = 0xab,
    OP_CHECKSIG = 0xac,
    OP_CHECKSIGVERIFY = 0xad,
    OP_CHECKMULTISIG = 0xae,
    OP_CHECKMULTISIGVERIFY = 0xaf,

    // expansion
    OP_NOP1 = 0xb0,
    OP_CHECKLOCKTIMEVERIFY = 0xb1,
    OP_NOP2 = OP_CHECKLOCKTIMEVERIFY,
    OP_CHECKSEQUENCEVERIFY = 0xb2,
    OP_NOP3 = OP_CHECKSEQUENCEVERIFY,
    OP_NOP4 = 0xb3,
    OP_NOP5 = 0xb4,
    OP_NOP6 = 0xb5,
    OP_NOP7 = 0xb6,
    OP_NOP8 = 0xb7,
    OP_NOP9 = 0xb8,
    OP_NOP10 = 0xb9,


    // template matching params
    OP_SMALLINTEGER = 0xfa,
    OP_PUBKEYS = 0xfb,
    OP_PUBKEYHASH = 0xfd,
    OP_PUBKEY = 0xfe,

    OP_INVALIDOPCODE = 0xff,
};

class ScriptInt : public ArithType<int64_t> {
public:
	explicit ScriptInt(const int64_t& n)
	{
		set_value(n);
	}
	explicit ScriptInt(const std::vector<uint8_t>&, bool);
	
	//-------------------------------------------------------------------------
	int IntValue() const
	{
		if (value() > INT32_MAX)
			return INT32_MAX;
		if (value() < INT32_MIN)
			return INT32_MIN;
		return value();
	}
	std::vector<uint8_t> BytesValue() const
	{
		return BytesEncoding(value());
	}
	static std::vector<uint8_t> BytesEncoding(const uint64_t&);
	
private:
	static constexpr std::size_t max_size = 4;
};

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

class Script {
public:
	Script() {}
	Script(std::vector<uint8_t>&& v)
		: data_(std::move(v)) {}
	Script(const std::vector<uint8_t>& v)
		: data_(v) {}
	Script(Script&& s)
		: data_(std::move(s.data_)) {}
	Script(const Script& s)
		: data_(s.data_) {}
	
	//-------------------------------------------------------------------------
	template <typename SType>
	void Serialize(SType& os) const
	{
		Serializer<SType> serial(os);
		serial.SerialWrite(data_);
	}
	template <typename SType>
	void UnSerialize(SType& is)
	{
		Serializer<SType> serial(is);
		serial.SerialRead(&data_);
	}
	
	//-------------------------------------------------------------------------
	void Push(const uint64_t& b)
	{
		if (b == -1 || (b >= 1 && b <= 16))
			data_.push_back(b + static_cast<uint8_t>(Opcode::OP_1) - 1);
		else if (b == 0)
			data_.push_back(static_cast<uint8_t>(Opcode::OP_0));
		else
			this->Push(ScriptInt::BytesEncoding(b));
	}
	
	void Push(const Opcode& b)
	{
		data_.push_back(static_cast<uint8_t>(b));
	}
	
	void Push(const ScriptInt& b)
	{
		this->Push(b.BytesValue());
	}
	
	void Push(const std::vector<uint8_t>& b)
	{
		if (b.size() < static_cast<uint8_t>(Opcode::OP_PUSHDATA1))
			data_.push_back(static_cast<uint8_t>(b.size()));
		else if (b.size() <= 0xff) {
			data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA1));
			data_.push_back(static_cast<uint8_t>(b.size()));
		}
		else if (b.size() <= 0xffff) {
			data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA2));
			Bytes<sizeof(uint16_t)> bytes;
			ToLittleEndian(static_cast<uint16_t>(b.size()), &bytes);
			data_.insert(data_.end(), bytes.begin(), bytes.end());
		}
		else {
			data_.push_back(static_cast<uint8_t>(Opcode::OP_PUSHDATA4));
			Bytes<sizeof(uint32_t)> bytes;
			ToLittleEndian(static_cast<uint32_t>(b.size()), &bytes);
			data_.insert(data_.end(), bytes.begin(), bytes.end());
		}
		data_.insert(data_.end(), b.begin(), b.end());
	}
	
	//-------------------------------------------------------------------------
	bool GetOpcode(std::vector<uint8_t>::const_iterator&, Opcode*) const;
	bool GetData(std::vector<uint8_t>::const_iterator&, const Opcode&, std::vector<uint8_t>*) const;
	
private:
	std::vector<uint8_t> data_;
};

bool Script::GetOpcode(std::vector<uint8_t>::const_iterator& pc, Opcode *out) const
{
	ASSERT_NULL(out);
	if (pc >= data_.end())
		return false;
	
	*out = static_cast<Opcode>(*pc);
	pc++;	
	
	return true;
}

bool Script::GetData(std::vector<uint8_t>::const_iterator& pc, const Opcode& in, std::vector<uint8_t> *out) const
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
		FromLittleEndian(pc, pc+2, &size);
		pc += 2;
	}
	else if (in == Opcode::OP_PUSHDATA4) {
		if (pc + 4 > data_.end())
			return false;
		FromLittleEndian(pc, pc+4, &size);
		pc += 4;
	}
	
	if (pc + size > data_.end())
		return false;
	out->assign(pc, pc+size);
	
	return true;
}

#endif // BTCLITE_SCRIPT_H
