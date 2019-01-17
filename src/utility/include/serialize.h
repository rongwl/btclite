#ifndef BTCLITE_SERIALIZE_H
#define BTCLITE_SERIALIZE_H

#include <map>
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
	
	template <typename T>
	void SerialWrite(const T& obj)
	{
		Serialize(stream_, obj);
	}
	
	template <typename T>
	void SerialRead(T& obj)
	{
		Unserialize(stream_, obj);
	}
	
private:
	SType& stream_;
	
	void Serialize(const double&); // for double type
	void Serialize(const float& in); // for float type
	template <typename T> void Serialize(const T&); // for integral type and other class
	
	void UnSerialize(double*); // for double type
	void UnSerialize(float*); // for float type
	template <typename T> void Unserialize(T*); // for integral type and other class
	
	/* Lowest-level serialization and conversion. */
	template <typename T> void SerWriteData(const T&);
	template <typename T> void SerReadData(T*);	
};

template <typename SType>
void Serializer<SType>::Serialize(const double& in)
{
	SerWriteData(DoubleToBinary(in));
}

template <typename SType>
void Serializer<SType>::Serialize(const float& in)
{
	SerWriteData(FloatToBinary(in));
}

template <typename SType>
template <typename T>
void Serializer<SType>::Serialize(const T& obj)
{
	if (std::is_integral<T>::value)
		SerWriteData(obj);
	else // default to calling member function
		obj.Serialize(stream_);
}

template <typename SType>
void Serializer<SType>::UnSerialize(double *out)
{
	uint64_t i;
	SerReadData(&i);
	*out = BinaryToDouble(i);		
}

template <typename SType>
void Serializer<SType>::UnSerialize(float *out)
{
	uint32_t i;
	SerReadData(&i);
	*out = BinaryToFloat(i);
}

template <typename SType>
template <typename T>
void Serializer<SType>::Unserialize(T *obj)
{
	if (std::is_integral<T>::value)
		SerReadData(obj);
	else // default to calling member function
		obj->Unserialize(stream_);
}

template <typename SType>
template <typename T>
void Serializer<SType>::SerWriteData(const T& obj)
{
	Bytes<sizeof(T)> data;
	ToLittleEndian(obj, &data);
	stream_.write(reinterpret_cast<char*>(&data[0]), sizeof data);
}

template <typename SType>
template <typename T>
void Serializer<SType>::SerReadData(T *obj)
{
	Bytes<sizeof(T)> data;
	stream_.read(reinterpret_cast<char*>(&data[0]), sizeof data);
	FromLittleEndian(data, obj);
}


#endif // BTCLITE_SERIALIZE_H
