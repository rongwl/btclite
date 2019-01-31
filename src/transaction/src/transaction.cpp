#include <botan/hash.h>
#include <numeric>

#include "transaction.h"

std::string TxIn::ToString() const
{
    std::string str;
    str += "TxIn(";
    str += prevout_.ToString();
    if (prevout_.IsNull())
        str += strprintf(", coinbase %s", HexEncode(script_sig_.begin(), script_sig_.end()));
    else
        str += strprintf(", scriptSig=%s", HexEncode(script_sig_.begin(), script_sig_.end()).substr(0, 24));
    if (sequence_no_ != default_sequence_no)
        str += strprintf(", sequence_no=%u", sequence_no_);
    str += ")";
	
    return str;
}

template <typename SType>
void Transaction::Serialize(SType& os) const
{
	Serializer<SType> serial(os);
	serial.SerialWrite(version_);
	serial.SerialWrite(inputs_);
	serial.SerialWrite(outputs_);
	serial.SerialWrite(lock_time_);
}

template <typename SType>
void Transaction::UnSerialize(SType& is)
{
	Serializer<SType> serial(is);
	serial.SerialRead(&version_);
	serial.SerialRead(&inputs_);
	serial.SerialRead(&outputs_);
	serial.SerialRead(&lock_time_);
}

const Hash256& Transaction::Hash()
{
	std::stringstream ss;
	std::unique_ptr<Botan::HashFunction> hash_func(Botan::HashFunction::create("SHA-256"));
	
	Serialize(ss);
	hash_func->update(ss.str());
	hash_.SetNull();
	hash_func->final(reinterpret_cast<uint8_t*>(&hash_));
	
	return hash_;
}

uint64_t Transaction::OutputsAmount() const
{
	uint64_t amount = 0;
	for (auto output : outputs_) {
		amount += output.value();
		if (output.value() > max_satoshi_amount || amount > max_satoshi_amount)
			throw std::runtime_error(std::string(__func__) + ": value out of range");
	}
	
	return amount;
}

std::size_t Transaction::Size(bool serialized) const
{
    const auto ins = [serialized](std::size_t size, const TxIn& input)
    {
        return size + input.Size(serialized);
    };
    const auto outs = [serialized](std::size_t size, const TxOut& output)
    {
        return size + output.Size(serialized);
    };
	
	return sizeof(version_) + sizeof(lock_time_)
		   + VarIntSize(inputs_.size())
		   + std::accumulate(inputs_.begin(), inputs_.end(), std::size_t{0}, ins)
		   + VarIntSize(outputs_.size())
		   + std::accumulate(outputs_.begin(), outputs_.end(), std::size_t{0}, outs);
}

std::string Transaction::ToString() const
{
    std::string str;
    str += strprintf("Transaction(hash=%s, ver=%d, inputs.size=%u, outputs.size=%u, lock_time=%u)\n",
					 HashCache().ToString().substr(0,10), version_, inputs_.size(), outputs_.size(), lock_time_);
    for (const auto& tx_in : inputs_)
        str += "    " + tx_in.ToString() + "\n";
    /*for (const auto& tx_in : inputs_)
        str += "    " + tx_in.scriptWitness.ToString() + "\n";*/
    for (const auto& tx_out : outputs_)
        str += "    " + tx_out.ToString() + "\n";
	
    return str;
}

