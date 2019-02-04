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
	// for double type
	void Serialize(const double& in) 
	{
		SerWriteData(DoubleToBinary(in));
	}
	
	// for float type
	void Serialize(const float& in)
	{
		SerWriteData(FloatToBinary(in));
	}
	
	// for arithmetic type vector
	template <typename T>
	std::enable_if_t<std::is_arithmetic<T>::value> Serialize(const std::vector<T>& v)
	{
		SerWriteVarInt(v.size());
		if (!v.empty())
			stream_.write(reinterpret_cast<const char*>(v.data()), v.size()*sizeof(T));
	}
	
	// for class type vector
	template <typename T>
	std::enable_if_t<std::is_class<T>::value> Serialize(const std::vector<T>& v)
	{
		SerWriteVarInt(v.size());
		for (auto it = v.begin(); it != v.end(); it++)
			Serialize(*it);
	}
	
	// for integral type
	template <typename T>
	std::enable_if_t<std::is_integral<T>::value> Serialize(const T& in) 
	{
		SerWriteData(in);
	}
	
	// default to calling member function 
	template <typename T>
	std::enable_if_t<std::is_class<T>::value> Serialize(const T& obj) 
	{
		obj.Serialize(stream_);
	}
	
	// for double type
	void UnSerialize(double *out) 
	{
		uint64_t i;
		SerReadData(&i);
		*out = BinaryToDouble(i);		
	}
	
	// for float type
	void UnSerialize(float *out) 
	{
		uint32_t i;
		SerReadData(&i);
		*out = BinaryToFloat(i);
	}
	
	// for arithmetic type vector
	template <typename T> 
	std::enable_if_t<std::is_arithmetic<T>::value> UnSerialize(std::vector<T>*); 
	
	// for class type vector
	template <typename T> 
	std::enable_if_t<std::is_class<T>::value> UnSerialize(std::vector<T>*); 
	
	// for integral type
	template <typename T>
	std::enable_if_t<std::is_integral<T>::value> UnSerialize(T *out) 
	{
		SerReadData(out);
	}
	
	// default to calling member function
	template <typename T>
	std::enable_if_t<std::is_class<T>::value> UnSerialize(T *obj) 
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
std::enable_if_t<std::is_arithmetic<T>::value> Serializer<SType>::UnSerialize(std::vector<T> *out)
{
	// Limit size per read so bogus size value won't cause out of memory
	uint64_t count = SerReadVarInt();
	if (count*sizeof(T) > max_block_size)
		throw std::ios_base::failure("vector size larger than max block size");
	out->clear();
	out->resize(count);
	stream_.read(reinterpret_cast<char*>(&out->data()), count*sizeof(T));
}

template <typename SType>
template <typename T> 
std::enable_if_t<std::is_class<T>::value> Serializer<SType>::UnSerialize(std::vector<T> *out)
{
	uint64_t count = SerReadVarInt();
	std::size_t size = 0;	
	out->clear();
	out->resize(count);
	for (uint64_t i = 0; i < count; i++) {
		UnSerialize(&(out->at(i)));
		size += out->at(i).Size(true);
		if (size > max_block_size)
			throw std::ios_base::failure("vector size larger than max block size");
	}
}

template <typename SType>
void Serializer<SType>::SerWriteVarInt(const uint64_t count)
{
	if (count < varint_16bits) {
		SerWriteData(static_cast<uint8_t>(count));
	}
	else if (count <= UINT16_MAX) {
		SerWriteData(varint_16bits);
		SerWriteData(static_cast<uint16_t>(count));
	}
	else if (count <= UINT32_MAX) {
		SerWriteData(varint_32bits);
		SerWriteData(static_cast<uint32_t>(count));
	}
	else {
		SerWriteData(varint_64bits);
		SerWriteData(count);
	}
}

template <typename SType>
uint64_t Serializer<SType>::SerReadVarInt()
{
	uint8_t count;
	uint64_t varint;
	
	SerReadData(&count);
	if (count < varint_16bits) {
		varint = count;
	}
	else if (count == varint_16bits) {
		SerReadData(reinterpret_cast<uint16_t*>(&varint));
		if (varint < varint_16bits)
			throw std::ios_base::failure("non-canonical SerReadVarInt()");
	}
	else if (count == varint_32bits) {
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