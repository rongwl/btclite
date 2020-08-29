#include "transaction.h"

#include <botan/hash.h>
#include <numeric>


namespace btclite {
namespace consensus {

OutPoint::OutPoint()
    : index_(std::numeric_limits<uint32_t>::max()) 
{
}
    
OutPoint::OutPoint(const util::Hash256& hash, uint32_t index)
    : prev_hash_(hash), index_(index)
{
}

OutPoint::OutPoint(util::Hash256&& hash, uint32_t index) noexcept
    : prev_hash_(std::move(hash)), index_(index) 
{
}
    
OutPoint::OutPoint(const OutPoint& op)
    : prev_hash_(op.prev_hash_), index_(op.index_) 
{
}

OutPoint::OutPoint(OutPoint&& op) noexcept
    : prev_hash_(std::move(op.prev_hash_)), index_(op.index_) 
{
}

void OutPoint::Clear()
{
    prev_hash_.fill(0);
    index_ = std::numeric_limits<uint32_t>::max();
}

bool OutPoint::IsNull() const
{
    return (prev_hash_ == crypto::null_hash &&
            index_ == std::numeric_limits<uint32_t>::max());
}

size_t OutPoint::Size() const
{
    return prev_hash_.size() + sizeof(index_);
}

std::string OutPoint::ToString() const
{
    std::stringstream ss;
    ss << "OutPoint(" 
       << util::EncodeHex(prev_hash_.rbegin(), prev_hash_.rend()).substr(0, 10) 
       << ", " << index_ << ")";
    return ss.str();
}

OutPoint& OutPoint::operator=(const OutPoint& b)
{
    prev_hash_ = b.prev_hash_;
    index_ = b.index_;
    return *this;
}

OutPoint& OutPoint::operator=(OutPoint&& b) noexcept
{
    if (this != &b) {
        prev_hash_ = std::move(b.prev_hash_);
        index_ = b.index_;
    }
    return *this;
}

const util::Hash256& OutPoint::prev_hash() const
{
    return prev_hash_;
}

void OutPoint::set_prevHash(const util::Hash256& hash)
{
    prev_hash_ = hash;
}

void OutPoint::set_prevHash(util::Hash256&& hash)
{
    prev_hash_ = std::move(hash);
}

uint32_t OutPoint::index() const
{
    return index_;
}

void OutPoint::set_index(uint32_t index)
{
    index_ = index;
}

TxIn::TxIn()
    : sequence_no_(default_sequence_no)
{
}
      
TxIn::TxIn(const OutPoint& prevout, const Script& script_sig, 
           uint32_t sequence_no, const ScriptWitness& script_witness)
    : prevout_(prevout), script_sig_(script_sig),
      sequence_no_(sequence_no), script_witness_(script_witness) 
{
}

TxIn::TxIn(OutPoint&& prevout, Script&& script_sig, 
           uint32_t sequence_no, ScriptWitness&& script_witness) noexcept
    : prevout_(std::move(prevout)), script_sig_(std::move(script_sig)),
      sequence_no_(sequence_no), script_witness_(std::move(script_witness))
{
}
      
TxIn::TxIn(const TxIn& input)
    : prevout_(input.prevout_), script_sig_(input.script_sig_), 
      sequence_no_(input.sequence_no_), script_witness_(input.script_witness_) 
{
}

TxIn::TxIn(TxIn&& input) noexcept
    : prevout_(std::move(input.prevout_)), script_sig_(std::move(input.script_sig_)),
      sequence_no_(input.sequence_no_), script_witness_(std::move(input.script_witness_))
{
}

bool TxIn::operator==(const TxIn& b) const
{
    return (prevout_ == b.prevout_ &&
            script_sig_ == b.script_sig_ &&
            sequence_no_ == b.sequence_no_);
}

bool TxIn::operator!=(const TxIn& b) const
{
    return !(*this == b);
}

TxIn& TxIn::operator=(const TxIn& b)
{
    prevout_ = b.prevout_;
    script_sig_ = b.script_sig_;
    sequence_no_ = b.sequence_no_;
    return *this;
}

TxIn& TxIn::operator=(TxIn&& b) noexcept
{
    if (this != &b) {
        prevout_ = std::move(b.prevout_);
        script_sig_ = std::move(b.script_sig_);
        sequence_no_ = b.sequence_no_;
    }
    return *this;
}

std::string TxIn::ToString() const
{
    std::stringstream ss;
    ss << "TxIn(" << prevout_.ToString() << ", ";
    if (prevout_.IsNull()) {
        ss << "scriptsig=" << util::EncodeHex(
               script_sig_.begin(), script_sig_.end());
    }
    else {
        ss << "scriptsig=" << util::EncodeHex(
               script_sig_.begin(), script_sig_.end()).substr(0, 24);
    }
    if (sequence_no_ != default_sequence_no) {
        ss << ", sequence_no=" << sequence_no_;
    }
    ss << ")";
    
    return ss.str();
}

size_t TxIn::SerializedSize() const
{
    return prevout_.Size() + script_sig_.SerializedSize() + sizeof(sequence_no_);
}

bool TxIn::HasWitness() const
{
    return false;
}

const OutPoint& TxIn::prevout() const
{
    return prevout_;
}

void TxIn::set_prevout(const OutPoint& prevout)
{
    prevout_ = prevout;
}

void TxIn::set_prevout(const OutPoint&& prevout)
{
    prevout_ = std::move(prevout);
}

const Script& TxIn::script_sig() const
{
    return script_sig_;
}

void TxIn::set_scriptSig(const Script& script)
{
    script_sig_ = script;
}

void TxIn::set_scriptSig(const Script&& script)
{
    script_sig_ = std::move(script);
}

uint32_t TxIn::sequence_no() const
{
    return sequence_no_;
}

void TxIn::set_sequenceNo(uint32_t sequence)
{
    sequence_no_ = sequence;
}

const ScriptWitness& TxIn::script_witness() const
{
    return script_witness_;
}

void TxIn::set_scriptWitness(const ScriptWitness& script_witness)
{
    script_witness_ = script_witness;
}

void TxIn::set_scriptWitness(ScriptWitness&& script_witness)
{
    script_witness_ = std::move(script_witness);
}

TxOut::TxOut()
    : value_(null_value)
{
    script_pub_key_.clear();
}
    
TxOut::TxOut(uint64_t value, const Script& script)
    : value_(value), script_pub_key_(script) 
{
}

TxOut::TxOut(uint64_t value, Script&& script) noexcept
    : value_(value), script_pub_key_(std::move(script)) 
{
}
    
TxOut::TxOut(const TxOut& output)
    : value_(output.value_), script_pub_key_(output.script_pub_key_)
{
}

TxOut::TxOut(TxOut&& output) noexcept
    : value_(output.value_), script_pub_key_(std::move(output.script_pub_key_)) 
{
}

void TxOut::Clear()
{
    value_ = null_value;
    script_pub_key_.clear();
}

bool TxOut::IsNull()
{
    return value_ == null_value;
}

std::string TxOut::ToString() const
{
    std::stringstream ss;
    ss << "TxOut(value=" << (value_ / kSatoshiPerBitcoin) << "."
       << std::setw(8) << std::setfill('0') << (value_ % kSatoshiPerBitcoin) << ", "
       << "scriptPubKey=" 
       << util::EncodeHex(script_pub_key_.begin(),
               script_pub_key_.end()) 
       << ")";
    return ss.str();
}

size_t TxOut::SerializedSize() const
{
    return script_pub_key_.SerializedSize() + sizeof(value_);
}

bool TxOut::operator==(const TxOut& b) const
{
    return (value_ == b.value_ &&
            script_pub_key_ == b.script_pub_key_);
}

bool TxOut::operator!=(const TxOut& b) const
{
    return !(*this == b);
}

TxOut& TxOut::operator=(const TxOut& b)
{
    value_ = b.value_;
    script_pub_key_ = b.script_pub_key_;
    return *this;
}

TxOut& TxOut::operator=(TxOut&& b) noexcept
{
    if (this != &b) {
        value_ = b.value_;
        script_pub_key_ = std::move(b.script_pub_key_);
    }
    return *this;
}

uint64_t TxOut::value() const
{
    return value_;
}

void TxOut::set_value(uint64_t value)
{
    value_ = value;
}

const Script& TxOut::script_pub_key() const
{
    return script_pub_key_;
}

void TxOut::set_scriptPubKey(const Script& script)
{
    script_pub_key_ = script;
}

void TxOut::set_scriptPubKey(Script&& script)
{
    script_pub_key_ = std::move(script);
}

Transaction::Transaction()
    : version_(default_version), inputs_(), outputs_(), lock_time_(0), hash_cache_()
{
}

Transaction::Transaction(uint32_t version, const std::vector<TxIn>& inputs,
            const std::vector<TxOut>& outputs, uint32_t lock_time)
    : version_(version), inputs_(inputs), outputs_(outputs), lock_time_(lock_time)
{
    GetHash();
}

Transaction::Transaction(uint32_t version, std::vector<TxIn>&& inputs,
            std::vector<TxOut>&& outputs, uint32_t lock_time) noexcept
    : version_(version), inputs_(std::move(inputs)),
      outputs_(std::move(outputs)), lock_time_(lock_time)
{
    GetHash();
}

Transaction::Transaction(const Transaction& t)
    : version_(t.version_), inputs_(t.inputs_), outputs_(t.outputs_),
      lock_time_(t.lock_time_)
{
    GetHash();
}

Transaction::Transaction(Transaction&& t) noexcept
    : version_(t.version_), inputs_(std::move(t.inputs_)), 
      outputs_(std::move(t.outputs_)), lock_time_(t.lock_time_)
{
    GetHash();
}

template <typename Stream>
void Transaction::Serialize(Stream& os, bool witness) const
{
    util::Serializer<Stream> serializer(os);
    serializer.SerialWrite(version_);
    serializer.SerialWrite(inputs_);
    serializer.SerialWrite(outputs_);
    serializer.SerialWrite(lock_time_);
}

template <typename Stream>
void Transaction::Deserialize(Stream& is, bool witness)
{
    util::Deserializer<Stream> deserializer(is);
    deserializer.SerialRead(&version_);
    deserializer.SerialRead(&inputs_);
    deserializer.SerialRead(&outputs_);
    deserializer.SerialRead(&lock_time_);
}

bool Transaction::operator==(const Transaction& b) const
{
    return this->GetHash() == b.GetHash();
}

bool Transaction::operator!=(const Transaction& b) const
{
    return !(*this == b);
}

Transaction& Transaction::operator=(const Transaction& b)
{
    version_ = b.version_;
    inputs_ = b.inputs_;
    outputs_ = b.outputs_;
    lock_time_ = b.lock_time_;
    hash_cache_.fill(0);
    
    return *this;
}

Transaction& Transaction::operator=(Transaction&& b) noexcept
{
    if (this != &b) {
        version_ = b.version_;
        inputs_ = std::move(b.inputs_);
        outputs_ = std::move(b.outputs_);
        lock_time_ = std::move(b.lock_time_);
        hash_cache_.fill(0);
    }
    
    return *this;
}

util::Hash256 Transaction::GetHash() const
{
    if (hash_cache_ == crypto::null_hash) {
        crypto::HashOStream hs;
        hs << *this;
        hash_cache_ = hs.DoubleSha256();
    }
    
    return hash_cache_;
}

util::Hash256 Transaction::GetWitnessHash() const
{
    return witness_hash_cache_;
}

bool Transaction::IsNull() const
{
    return inputs_.empty() && outputs_.empty();
}

bool Transaction::IsCoinBase() const
{
    return (inputs_.size() == 1 && inputs_[0].prevout().IsNull());
}

uint64_t Transaction::OutputsAmount() const
{
    uint64_t amount = 0;
    for (auto output : outputs_) {
        amount += output.value();
        if (output.value() > kMaxSatoshiAmount || amount > kMaxSatoshiAmount)
            throw std::runtime_error(std::string(__func__) + ": value out of range");
    }
    
    return amount;
}

size_t Transaction::SerializedSize() const
{
    const auto ins = [](size_t size, const TxIn& input)
    {
        return size + input.SerializedSize();
    };
    const auto outs = [](size_t size, const TxOut& output)
    {
        return size + output.SerializedSize();
    };
    
    return sizeof(version_) + sizeof(lock_time_)
           + util::VarIntSize(inputs_.size())
           + std::accumulate(inputs_.begin(), inputs_.end(), size_t{0}, ins)
           + util::VarIntSize(outputs_.size())
           + std::accumulate(outputs_.begin(), outputs_.end(), size_t{0}, outs);
}

std::string Transaction::ToString() const
{
    std::stringstream ss;
    util::Hash256 hash = GetHash();
    ss << "Transaction(hash=" 
       << util::EncodeHex(hash.rbegin(), hash.rend()).substr(0,10) << ", "
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

uint32_t Transaction::version() const
{
    return version_;
}

void Transaction::set_version(uint32_t v)
{
    version_ = v;
    hash_cache_.fill(0);
}

const std::vector<TxIn>& Transaction::inputs() const
{
    return inputs_;
}

void Transaction::set_inputs(const std::vector<TxIn>& inputs)
{
    inputs_ = inputs;
    hash_cache_.fill(0);
}

void Transaction::set_inputs(std::vector<TxIn>&& inputs)
{
    inputs_ = std::move(inputs);
    hash_cache_.fill(0);
}

const std::vector<TxOut>& Transaction::outputs() const
{
    return outputs_;
}

void Transaction::set_outputs(const std::vector<TxOut>& outputs)
{
    outputs_ = outputs;
    hash_cache_.fill(0);
}

void Transaction::set_outputs(std::vector<TxOut>&& outputs)
{
    outputs_ = std::move(outputs);
    hash_cache_.fill(0);
}

uint32_t Transaction::lock_time() const
{
    return lock_time_;
}

void Transaction::set_lockTime(uint32_t t)
{
    lock_time_ = t;
    hash_cache_.fill(0);
}

} // namespace consensus
} // namespace btclite
