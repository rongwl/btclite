#include <botan/hash.h>
#include <numeric>

#include "transaction.h"

std::string TxIn::ToString() const
{
	std::stringstream ss;
	ss << "TxIn(" << prevout_.ToString() << ", ";
	if (prevout_.IsNull())
		ss << "coinbase=" << HexEncode(script_sig_.begin(), script_sig_.end());
	else
		ss << "scriptSig=" << HexEncode(script_sig_.begin(), script_sig_.end()).substr(0, 24);
	if (sequence_no_ != default_sequence_no)
		ss << ", sequence_no=" << sequence_no_;
	ss << ")";
	
	return ss.str();
}

template <typename Stream>
void Transaction::Serialize(Stream& os, bool witness) const
{
	Serializer<Stream> serial(os);
	serial.SerialWrite(version_);
	serial.SerialWrite(inputs_);
	serial.SerialWrite(outputs_);
	serial.SerialWrite(lock_time_);
}

template <typename Stream>
void Transaction::UnSerialize(Stream& is, bool witness)
{
	Serializer<Stream> serial(is);
	serial.SerialRead(&version_);
	serial.SerialRead(&inputs_);
	serial.SerialRead(&outputs_);
	serial.SerialRead(&lock_time_);
}

Transaction& Transaction::operator=(const Transaction& b)
{
	version_ = b.version_;
	inputs_ = b.inputs_;
	outputs_ = b.outputs_;
	lock_time_ = b.lock_time_;
	hash_cache_.SetNull();
	
	return *this;
}

Transaction& Transaction::operator=(Transaction&& b) noexcept
{
	if (this != &b) {
		version_ = b.version_;
		inputs_ = std::move(b.inputs_);
		outputs_ = std::move(b.outputs_);
		lock_time_ = std::move(b.lock_time_);
		hash_cache_.SetNull();
	}
	
	return *this;
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

std::size_t Transaction::Size(bool serialized = false) const
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
	std::stringstream ss;
	ss << "Transaction(hash=" << Hash().ToString().substr(0,10) << ", "
	   << "ver=" << version_ << ", "
	   << "inputs.size=" << inputs_.size() << ", "
	   << "outputs.size=" << outputs_.size() << ", "
	   << "lock_time=" << lock_time_ << ")\n";
	for (const auto& tx_in : inputs_)
		ss << "    " << tx_in.ToString() << "\n";
	/*for (const auto& tx_in : inputs_)
        ss << "    " << tx_in.scriptWitness.ToString() << "\n";*/
	for (const auto& tx_out : outputs_)
        ss << "    " << tx_out.ToString() << "\n";
	
	return ss.str();
}

const Hash256& Transaction::Hash() const
{
	if (hash_cache_.IsNull()) {
		std::vector<uint8_t> v;
		ByteSink<std::vector<uint8_t> > byte_sink(v);
		Serialize(byte_sink, false);
		DoubleSha256(v, &hash_cache_);
		// Botan hash output is big endian, Hash256 is little endian
		ReverseEndian(hash_cache_.begin(), hash_cache_.end());
	}
	return hash_cache_;
}

const Hash256& Transaction::WitnessHash() const
{
	return witness_hash_cache_;
}
