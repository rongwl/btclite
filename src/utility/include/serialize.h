#ifndef BTCLITE_SERIALIZE_H
#define BTCLITE_SERIALIZE_H

#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <google/protobuf/repeated_field.h>
#include <map>

#include "constants.h"
#include "Endian.h"


namespace btclite {
namespace utility {
namespace serialize {

size_t VarIntSize(size_t vec_size);

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

} // namespace serialize
} // namespace utility
} // namespace btclite

template <typename Stream>
class Serializer {
public:
    Serializer(Stream& s)
        : stream_(s) {}
    
    // interface for serialize
    template <typename T>
    void SerialWrite(const T& obj)
    {
        Serialize(obj);
    }
    
private:
    Stream& stream_;
    
    // for double
    void Serialize(const double& in) 
    {
        SerWriteData(btclite::utility::serialize::DoubleToBinary(in));
    }
    
    // for float
    void Serialize(const float& in)
    {
        SerWriteData(btclite::utility::serialize::FloatToBinary(in));
    }
        
    //for string
    void Serialize(const std::string& in)
    {
        SerWriteVarInt(in.size());
        stream_.write(reinterpret_cast<const char*>(in.data()), in.size());
    }
    
    // for integral
    template <typename T>
    std::enable_if_t<std::is_integral<T>::value> Serialize(const T& in) 
    {
        SerWriteData(in);
    }
    
    //for arithmetic std::array
    template <typename T, size_t N>
    std::enable_if_t<std::is_arithmetic<T>::value> Serialize(const std::array<T, N>& in)
    {
        stream_.write(reinterpret_cast<const char*>(in.data()), N*sizeof(T));
    }
    
    // for arithmetic vector
    template <typename T>
    std::enable_if_t<std::is_arithmetic<T>::value> Serialize(const std::vector<T>& in)
    {
        SerWriteVarInt(in.size());
        if (!in.empty())
            stream_.write(reinterpret_cast<const char*>(in.data()), in.size()*sizeof(T));
    }
    
    // for string vector
    void Serialize(const std::vector<std::string>& in)
    {
        SerWriteVarInt(in.size());
        for (auto it = in.begin(); it != in.end(); it++)
            Serialize(*it);
    }
    
    // for class vector
    template <typename T>
    std::enable_if_t<std::is_class<T>::value> Serialize(const std::vector<T>& in)
    {
        SerWriteVarInt(in.size());
        for (auto it = in.begin(); it != in.end(); it++)
            it->Serialize(stream_);
    }
    
    // for integral RepeatedField
    template <typename T>
    std::enable_if_t<std::is_integral<T>::value> Serialize(const ::google::protobuf::RepeatedField<T>& in)
    {
        stream_.write(reinterpret_cast<const char*>(in.data()), in.size()*sizeof(T));
    }
    
    // default to calling member function 
    template <typename T>
    std::enable_if_t<std::is_class<T>::value> Serialize(const T& obj) 
    {
        obj.Serialize(stream_);
    }
    
    // serialize variable length integer
    void SerWriteVarInt(const uint64_t);
    
    // Lowest-level serialization and conversion.
    template <typename T> void SerWriteData(const T&);
};

template <typename Stream>
void Serializer<Stream>::SerWriteVarInt(const uint64_t count)
{
    if (count < kVarint16bits) {
        SerWriteData(static_cast<uint8_t>(count));
    }
    else if (count <= std::numeric_limits<uint16_t>::max()) {
        SerWriteData(kVarint16bits);
        SerWriteData(static_cast<uint16_t>(count));
    }
    else if (count <= std::numeric_limits<uint32_t>::max()) {
        SerWriteData(kVarint32bits);
        SerWriteData(static_cast<uint32_t>(count));
    }
    else {
        SerWriteData(kVarint64bits);
        SerWriteData(count);
    }
}

template <typename Stream>
template <typename T>
void Serializer<Stream>::SerWriteData(const T& obj)
{
    Bytes<sizeof(T)> data;
    ToLittleEndian(obj, &data);
    stream_.write(reinterpret_cast<const char*>(&data[0]), sizeof data);
}

template <typename Stream>
class Deserializer {
public:
    Deserializer(Stream& s)
        : stream_(s) {}
    
    // interface for unserialize
    template <typename T>
    void SerialRead(T *obj)
    {
        Deserialize(obj);
    }
    
private:
    Stream& stream_;
    
    //-------------------------------------------------------------------------
    // for double
    void Deserialize(double *out) 
    {
        uint64_t i;
        SerReadData(&i);
        *out = btclite::utility::serialize::BinaryToDouble(i);        
    }
    
    // for float
    void Deserialize(float *out) 
    {
        uint32_t i;
        SerReadData(&i);
        *out = btclite::utility::serialize::BinaryToFloat(i);
    }
    
    // for string
    void Deserialize(std::string *out);
    
    // for integral type
    template <typename T>
    std::enable_if_t<std::is_integral<T>::value> Deserialize(T *out) 
    {
        SerReadData(out);
    }
    
    // for arithmetic std::array
    template <typename T, size_t N>
    std::enable_if_t<std::is_arithmetic<T>::value> Deserialize(std::array<T, N> *out)
    {
        for (auto it = out->begin(); it != out->end(); it++)
            Deserialize(&(*it));
    }
    
    // for arithmetic vector
    template <typename T> 
    std::enable_if_t<std::is_arithmetic<T>::value> Deserialize(std::vector<T> *out); 
    
    // for string vector
    void Deserialize(std::vector<std::string> *out);
    
    // for class vector
    template <typename T> 
    std::enable_if_t<std::is_class<T>::value> Deserialize(std::vector<T> *out); 
    
    // for integral RepeatedField
    template <typename T>
    std::enable_if_t<std::is_integral<T>::value> Deserialize(::google::protobuf::RepeatedField<T> *out)
    {
        for (uint8_t *p = reinterpret_cast<uint8_t*>(out->begin()); p != reinterpret_cast<uint8_t*>(out->end()); p++)
            Deserialize(p);
    }
    
    // default to calling member function
    template <typename T>
    std::enable_if_t<std::is_class<T>::value> Deserialize(T *obj) 
    {
        obj->Deserialize(stream_);
    }
    
    // deserialize variable length integer
    uint64_t SerReadVarInt();
    
    // Lowest-level deserialization and conversion.
    template <typename T> void SerReadData(T*);
};

template <typename Stream>
void Deserializer<Stream>::Deserialize(std::string *out)
{
    size_t size = SerReadVarInt();
    char c;
    
    // Read all size characters, pushing all non-null (may be many).
    out->clear();
    out->reserve(size);
    for (size_t i = 0; i < size; i++) {
        stream_.read(&c, 1);
        if (c == '\0')
            break;
        out->push_back(c);
    }
}

template <typename Stream>
template <typename T>
std::enable_if_t<std::is_arithmetic<T>::value> Deserializer<Stream>::Deserialize(std::vector<T> *out)
{
    // Limit size per read so bogus size value won't cause out of memory
    uint64_t count = SerReadVarInt();
    if (count*sizeof(T) > kMaxBlockSize)
        throw std::ios_base::failure("vector size larger than max block size");
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); it++)
        Deserialize(&(*it));
}

template <typename Stream>
void Deserializer<Stream>::Deserialize(std::vector<std::string> *out)
{
    uint64_t count = SerReadVarInt();
    size_t size = 0;    
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); it++) {
        Deserialize(&(*it));
        size += it->size();
        if (size > kMaxBlockSize)
            throw std::ios_base::failure("vector size larger than max block size");
    }
}

template <typename Stream>
template <typename T> 
std::enable_if_t<std::is_class<T>::value> Deserializer<Stream>::Deserialize(std::vector<T> *out)
{
    uint64_t count = SerReadVarInt();
    size_t size = 0;    
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); it++) {
        it->Deserialize(stream_);
        size += it->SerializedSize();
        if (size > kMaxBlockSize)
            throw std::ios_base::failure("vector size larger than max block size");
    }
}

template <typename Stream>
uint64_t Deserializer<Stream>::SerReadVarInt()
{
    uint8_t count;
    uint64_t varint;
    
    SerReadData(&count);
    if (count < kVarint16bits) {
        varint = count;
    }
    else if (count == kVarint16bits) {
        SerReadData(reinterpret_cast<uint16_t*>(&varint));
        if (varint < kVarint16bits)
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
    }
    else if (count == kVarint32bits) {
        SerReadData(reinterpret_cast<uint32_t*>(&varint));
        if (varint <= std::numeric_limits<uint16_t>::max())
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
    }
    else {
        SerReadData(&varint);
        if (varint <= std::numeric_limits<uint32_t>::max())
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
    }
    if (varint > kMaxVardataSize)
        throw std::ios_base::failure("SerReadVarInt(): size too large");
    
    return varint;
}

template <typename Stream>
template <typename T>
void Deserializer<Stream>::SerReadData(T *obj)
{
    Bytes<sizeof(T)> data;
    stream_.read(reinterpret_cast<char*>(&data[0]), sizeof data);
    FromLittleEndian(data.begin(), data.end(), obj);
}


#endif // BTCLITE_SERIALIZE_H
