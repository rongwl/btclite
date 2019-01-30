#ifndef BTCLITE_TRANSACTION_H
#define BTCLITE_TRANSACTION_H

#include "hash.h"
#include "serialize.h"
#include "tinyformat.h"

/** An outpoint - a combination of a transaction hash and an index n into its vout */
class OutPoint {
public:
	OutPoint()
		: index_(UINT32_MAX) {}
	OutPoint(const Hash256& hash, uint32_t index)
		: hash_(hash), index_(index) {}
	
	//-------------------------------------------------------------------------
	void SetNull()
	{
		hash_.SetNull();
		index_ = UINT32_MAX;
	}
	bool IsNull() const
	{
		return (hash_.IsNull() && index_ == UINT32_MAX);
	}
	
	//-------------------------------------------------------------------------
	friend bool operator==(const OutPoint& a, const OutPoint& b)
	{
		return (a.hash_ == b.hash_ && a.index_ == b.index_);
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
	
	//-------------------------------------------------------------------------
	template <typename SType>
	void Serialize(SType& os) const
	{
		Serializer<SType> serial(os);
		serial.SerialWrite(hash_);
		serial.SerialWrite(index_);
	}
	template <typename SType>
	void UnSerialize(SType& is)
	{
		Serializer<SType> serial(is);
		serial.SerialRead(&hash_);
		serial.SerialRead(&index_);
	}
	
	//-------------------------------------------------------------------------
	std::string ToString() const
	{
		return strprintf("OutPoint(%s, %u)", hash_.ToString().substr(0,10), index_);
	}
private:
	Hash256 hash_;
	uint32_t index_;
};


#endif // BTCLITE_TRANSACTION_H
