#ifndef BTCLITE_SERIALIZE_H
#define BTCLITE_SERIALIZE_H

#include <algorithm>
#include <cstdint>
#include <map>

#include "constants.h"
#include "Endian.h"

inline uint64_t DoubleToBinary(double d)
{
	union {
		double d;
		uint64_t i;
	} tmp;
	tmp.d = d;
	return tmp.i;
}

inline uint32_t FloatToBinary(float f)
{
	union {
		float f;
		uint32_t i;
	} tmp;
	tmp.f = f;
	return tmp.i;
}

inline double BinaryToDouble(uint64_t i)
{
	union {
		double d;
		uint64_t i;
	} tmp;
	tmp.i = i;
	return tmp.d;
}

inline float BinaryToFloat(uint32_t i)
{
	union {
		float f;
		uint32_t i;
	} tmp;
	tmp.i = i;
	return tmp.f;
}


template <typename SType>
class Serializer {
public:
	Serializer(SType& s)
		: stream_(s) {}
	
	// interface for serialize
	template <typename T>
	void SerialWrite(const T& obj)
	{
		Serialize(obj);
	}
	// interface for unserialize
	template <typename T>
	void SerialRead(T *obj)
	{
		Unserialize(obj);
	}
	
private:
	SType& stream_;
	
	//-------------------------------------------------------------------------
	void Serialize(const double& in) // for double type
	{
		SerWriteData(DoubleToBinary(in));
	}
	void Serialize(const float& in) // for float type
	{
		SerWriteData(FloatToBinary(in));
	}
	template <typename T> void Serialize(const std::vector<T>& v) // for vector
	{
		SerWriteVarInt(v.size());
		if (!v.empty())
			stream_.write(reinterpret_cast<const char*>(v.data()), v.size()*sizeof(T));
	}
	template <typename T>
	std::enable_if_t<std::is_integral<T>::value> Serialize(const T& in) // for integral type
	{
		SerWriteData(in);
	}
	template <typename T>
	std::enable_if_t<std::is_class<T>::value> Serialize(const T& obj) // default to calling member function 
	{
		obj.Serialize(stream_);
	}
	
	void UnSerialize(double *out) // for double type
	{
		uint64_t i;
		SerReadData(&i);
		*out = BinaryToDouble(i);		
	}
	void UnSerialize(float *out) // for float type
	{
		uint32_t i;
		SerReadData(&i);
		*out = BinaryToFloat(i);
	}	
	template <typename T> void UnSerialize(std::vector<T>*); // for vector
	template <typename T>
	std::enable_if_t<std::is_integral<T>::value> UnSerialize(T *out) // for integral type
	{
		SerReadData(out);
	}
	template <typename T>
	std::enable_if_t<std::is_class<T>::value> UnSerialize(T *obj) // default to calling member function
	{
		obj->UnSerialize(stream_);
	}
	
	// serialize variable length integer
	//-------------------------------------------------------------------------
	void SerWriteVarInt(const uint64_t);
	uint64_t SerReadVarInt();
	
	// Lowest-level serialization and conversion.
	//-------------------------------------------------------------------------
	template <typename T> void SerWriteData(const T&);
	template <typename T> void SerReadData(T*);		
};

template <typename SType>
template <typename T>
void Serializer<SType>::UnSerialize(std::vector<T> *out)
{
	// Limit size per read so bogus size value won't cause out of memory
	uint64_t size = SerReadVarInt();
	out->clear();
	uint64_t i;
	while (i < size) {
		uint32_t blk = std::min(size-i, static_cast<uint64_t>(1 + 4999999 / sizeof(T)));
		out->reserve(i+blk);
		stream_.read(reinterpret_cast<char*>(&out->data()), blk*sizeof(T));
		i += blk;
	}
}

template <typename SType>
void Serializer<SType>::SerWriteVarInt(const uint64_t nSize)
{
	if (nSize < varint_16bits) {
		SerWriteData(static_cast<uint8_t>(nSize));
	}
	else if (nSize <= UINT16_MAX) {
		SerWriteData(varint_16bits);
		SerWriteData(static_cast<uint16_t>(nSize));
	}
	else if (nSize <= UINT32_MAX) {
		SerWriteData(varint_32bits);
		SerWriteData(static_cast<uint32_t>(nSize));
	}
	else {
		SerWriteData(varint_64bits);
		SerWriteData(nSize);
	}
}

template <typename SType>
uint64_t Serializer<SType>::SerReadVarInt()
{
	uint8_t size;
	uint64_t varint;
	
	SerReadData(&size);
	if (size < varint_16bits) {
		varint = size;
	}
	else if (size == varint_16bits) {
		SerReadData(reinterpret_cast<uint16_t*>(&varint));
		if (varint < varint_16bits)
			throw std::ios_base::failure("non-canonical SerReadVarInt()");
	}
	else if (size == varint_32bits) {
		SerReadData(reinterpret_cast<uint32_t*>(&varint));
		if (varint <= UINT16_MAX)
			throw std::ios_base::failure("non-canonical SerReadVarInt()");
	}
	else {
		SerReadData(&varint);
		if (varint <= UINT32_MAX)
			throw std::ios_base::failure("non-canonical SerReadVarInt()");
	}
	if (varint > max_vardata_size)
		throw std::ios_base::failure("SerReadVarInt(): size too large");
	
	return varint;
}

template <typename SType>
template <typename T>
void Serializer<SType>::SerWriteData(const T& obj)
{
	Bytes<sizeof(T)> data;
	ToLittleEndian(obj, &data);
	stream_.write(reinterpret_cast<const char*>(&data[0]), sizeof data);
}

template <typename SType>
template <typename T>
void Serializer<SType>::SerReadData(T *obj)
{
	Bytes<sizeof(T)> data;
	stream_.read(reinterpret_cast<char*>(&data[0]), sizeof data);
	FromLittleEndian(data.begin(), data.end(), obj);
}


#endif // BTCLITE_SERIALIZE_H
