#ifndef BTCLITE_CONSENSUS_TRANSACTION_H
#define BTCLITE_CONSENSUS_TRANSACTION_H

#include "hash.h"
#include "script.h"
#include "script_witness.h"
#include "serialize.h"


namespace btclite {
namespace consensus {

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class OutPoint {
public:
    OutPoint();
    
    OutPoint(const util::Hash256& hash, uint32_t index);
    OutPoint(util::Hash256&& hash, uint32_t index) noexcept;
    
    OutPoint(const OutPoint& op);
    OutPoint(OutPoint&& op) noexcept;
    
    //-------------------------------------------------------------------------
    void Clear();
    bool IsNull() const;    
    size_t Size() const;
    std::string ToString() const;
    
    //-------------------------------------------------------------------------
    friend bool operator==(const OutPoint& a, const OutPoint& b)
    {
        return (a.prev_hash_ == b.prev_hash_ && a.index_ == b.index_);
    }
    
    friend bool operator!=(const OutPoint& a, const OutPoint& b)
    {
        return !(a == b);
    }
    
    friend bool operator<(const OutPoint& a, const OutPoint& b)
    {
        return (a < b || (a == b && a.index_ < b.index_));
    }
    
    friend bool operator>(const OutPoint& a, const OutPoint& b)
    {
        return (a > b || (a == b && a.index_ > b.index_));
    }
    
    OutPoint& operator=(const OutPoint& b);    
    OutPoint& operator=(OutPoint&& b) noexcept;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& os) const
    {
        util::Serializer<Stream> serializer(os);
        serializer.SerialWrite(prev_hash_);
        serializer.SerialWrite(index_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& is)
    {
        util::Deserializer<Stream> deserializer(is);
        deserializer.SerialRead(&prev_hash_);
        deserializer.SerialRead(&index_);
    }
    
    //-------------------------------------------------------------------------
    const util::Hash256& prev_hash() const;
    void set_prevHash(const util::Hash256& hash);
    void set_prevHash(util::Hash256&& hash);
    
    uint32_t index() const;
    void set_index(uint32_t index);
    
private:
    util::Hash256 prev_hash_;
    uint32_t index_;
};

/** An input of a transaction.  It contains the location of the previous
 * transaction's output that it claims and a signature that matches the
 * output's public key.
 */
class TxIn {
public:
    /* Setting nSequence to this value for every input in a transaction
    * disables nLockTime. */
    static constexpr uint32_t default_sequence_no = 0xffffffff;

    //-------------------------------------------------------------------------
    TxIn();
    
    TxIn(const OutPoint& prevout, const Script& script_sig, 
         uint32_t sequence_no=default_sequence_no,
         const ScriptWitness& script_witness = ScriptWitness());
    TxIn(OutPoint&& prevout, Script&& script_sig, 
         uint32_t sequence_no=default_sequence_no,
         ScriptWitness&& script_witness = ScriptWitness()) noexcept;
    
    TxIn(const TxIn& input);
    TxIn(TxIn&& input) noexcept;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& os) const
    {
        util::Serializer<Stream> serializer(os);
        serializer.SerialWrite(prevout_);
        serializer.SerialWrite(script_sig_);
        serializer.SerialWrite(sequence_no_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& is)
    {
        util::Deserializer<Stream> deserializer(is);
        deserializer.SerialRead(&prevout_);
        deserializer.SerialRead(&script_sig_);
        deserializer.SerialRead(&sequence_no_);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const TxIn& b) const; 
    bool operator!=(const TxIn& b) const;
    
    TxIn& operator=(const TxIn& b);
    TxIn& operator=(TxIn&& b) noexcept;
    
    //-------------------------------------------------------------------------
    std::string ToString() const;    
    size_t SerializedSize() const;    
    bool HasWitness() const;
    
    //-------------------------------------------------------------------------
    const OutPoint& prevout() const;
    void set_prevout(const OutPoint& prevout);
    void set_prevout(const OutPoint&& prevout);
    
    const Script& script_sig() const;
    void set_scriptSig(const Script& script);
    void set_scriptSig(const Script&& script);
    
    uint32_t sequence_no() const;
    void set_sequenceNo(uint32_t sequence);
    
    const ScriptWitness& script_witness() const;
    void set_scriptWitness(const ScriptWitness& script_witness);
    void set_scriptWitness(ScriptWitness&& script_witness);
    
private:
    OutPoint prevout_;
    Script script_sig_;
    uint32_t sequence_no_;
    ScriptWitness script_witness_;
};

/** An output of a transaction.  It contains the public key that the next input
 * must be able to sign with to claim it.
 */
class TxOut {
public:
    TxOut();
    
    TxOut(uint64_t value, const Script& script);
    TxOut(uint64_t value, Script&& script) noexcept;
    
    TxOut(const TxOut& output);
    TxOut(TxOut&& output) noexcept;
    
    //-------------------------------------------------------------------------
    void Clear();    
    bool IsNull();    
    std::string ToString() const;    
    size_t SerializedSize() const;
    
    //-------------------------------------------------------------------------
    template <typename Stream>
    void Serialize(Stream& os) const
    {
        util::Serializer<Stream> serializer(os);
        serializer.SerialWrite(value_);
        serializer.SerialWrite(script_pub_key_);
    }
    
    template <typename Stream>
    void Deserialize(Stream& is)
    {
        util::Deserializer<Stream> deserializer(is);
        deserializer.SerialRead(&value_);
        deserializer.SerialRead(&script_pub_key_);
    }
    
    //-------------------------------------------------------------------------
    bool operator==(const TxOut& b) const;
    bool operator!=(const TxOut& b) const;
    
    TxOut& operator=(const TxOut& b);
    TxOut& operator=(TxOut&& b) noexcept;
    
    //-------------------------------------------------------------------------
    uint64_t value() const;
    void set_value(uint64_t value);
    
    const Script& script_pub_key() const;
    void set_scriptPubKey(const Script& script);
    void set_scriptPubKey(Script&& script);
    
private:
    uint64_t value_;
    Script script_pub_key_;

    static constexpr uint64_t null_value = std::numeric_limits<uint64_t>::max();
};

class Transaction {
public:
    Transaction();
    
    Transaction(uint32_t version, const std::vector<TxIn>& inputs,
                const std::vector<TxOut>& outputs, uint32_t lock_time);
    Transaction(uint32_t version, std::vector<TxIn>&& inputs,
                std::vector<TxOut>&& outputs, uint32_t lock_time) noexcept;
        
    Transaction(const Transaction& t);    
    Transaction(Transaction&& t) noexcept;
    
    //-------------------------------------------------------------------------
    template <typename Stream> void Serialize(Stream& os, bool witness = false) const;
    template <typename Stream> void Deserialize(Stream& is, bool witness = false);
    
    //-------------------------------------------------------------------------
    bool operator==(const Transaction& b) const;
    bool operator!=(const Transaction& b) const;
    
    Transaction& operator=(const Transaction& b);
    Transaction& operator=(Transaction&& b) noexcept;
    
    //-------------------------------------------------------------------------
    util::Hash256 GetHash() const;
    util::Hash256 GetWitnessHash() const;
    
    //-------------------------------------------------------------------------
    bool IsNull() const;    
    bool IsCoinBase() const;
    
    size_t SerializedSize() const;
    uint64_t OutputsAmount() const;
    std::string ToString() const;
    
    //-------------------------------------------------------------------------
    uint32_t version() const;
    void set_version(uint32_t v);
    
    const std::vector<TxIn>& inputs() const;
    void set_inputs(const std::vector<TxIn>& inputs);
    void set_inputs(std::vector<TxIn>&& inputs);
    
    const std::vector<TxOut>& outputs() const;
    void set_outputs(const std::vector<TxOut>& outputs);
    void set_outputs(std::vector<TxOut>&& outputs);
    
    uint32_t lock_time() const;
    void set_lockTime(uint32_t t);
    
private:    
    uint32_t version_;
    std::vector<TxIn> inputs_;
    std::vector<TxOut> outputs_;
    uint32_t lock_time_;
    
    mutable util::Hash256 hash_cache_;
    mutable util::Hash256 witness_hash_cache_;
    
    // Default transaction version.
    static constexpr uint32_t default_version = 2;
};

} // namespace consensus
} // namespace btclite

#endif // BTCLITE_CONSENSUS_TRANSACTION_H
