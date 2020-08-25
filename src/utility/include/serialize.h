#ifndef BTCLITE_SERIALIZE_H
#define BTCLITE_SERIALIZE_H

#include <array>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <google/protobuf/repeated_field.h>
#include <map>

#include "constants.h"
#include "logging.h"
#include "thread.h"
#include "util_endian.h"


namespace btclite {
namespace util {

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
        SerWriteData(DoubleToBinary(in));
    }
    
    // for float
    void Serialize(const float& in)
    {
        SerWriteData(FloatToBinary(in));
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
    
    // for enum
    template <typename T>
    std::enable_if_t<std::is_enum<T>::value> Serialize(const T& in)
    {
        SerWriteData(static_cast<std::underlying_type_t<T> >(in));
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
        if (!in.empty()) {
            stream_.write(reinterpret_cast<const char*>(in.data()), in.size()*sizeof(T));
        }
    }
    
    // for string vector
    void Serialize(const std::vector<std::string>& in)
    {
        SerWriteVarInt(in.size());
        for (auto it = in.begin(); it != in.end(); it++) {
            Serialize(*it);
        }
    }
    
    // for class vector
    template <typename T>
    std::enable_if_t<std::is_class<T>::value> Serialize(const std::vector<T>& in)
    {
        SerWriteVarInt(in.size());
        for (auto it = in.begin(); it != in.end(); it++) {
            Serialize(*it);
        }
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
    uint8_t data[sizeof(T)];
    ToLittleEndian(obj, data);
    stream_.write(reinterpret_cast<const char*>(data), sizeof data);
}

template <typename Stream>
class Deserializer {
public:
    Deserializer(Stream& s)
        : stream_(s) {}
    
    // interface for Deserialize
    template <typename T>
    size_t SerialRead(T *obj)
    {
        return Deserialize(obj);
    }
    
private:
    Stream& stream_;
    
    //-------------------------------------------------------------------------
    size_t Deserialize(double *out); // for double
    size_t Deserialize(float *out); // for float
    size_t Deserialize(std::string *out); // for string
    
    // for integral type
    template <typename T>
    size_t Deserialize(T *out, std::enable_if_t<std::is_integral<T>::value>* = 0) 
    {
        return SerReadData(out);
    }
    
    // for enum
    template <typename T>
    size_t Deserialize(T *out, std::enable_if_t<std::is_enum_v<T> >* = 0)
    {
        return SerReadData(reinterpret_cast<std::underlying_type_t<T>*>(out));
    }
    
    // for arithmetic std::array
    template <typename T, size_t N>
    size_t Deserialize(std::array<T, N> *out, 
                       std::enable_if_t<std::is_arithmetic<T>::value>* = 0);
    
    // for arithmetic vector
    template <typename T> 
    size_t Deserialize(std::vector<T> *out, 
                       std::enable_if_t<std::is_arithmetic<T>::value>* = 0); 
    
    // for string vector
    size_t Deserialize(std::vector<std::string> *out);
    
    // for class vector
    template <typename T> 
    size_t Deserialize(std::vector<T> *out,
                       std::enable_if_t<std::is_class<T>::value>* = 0); 
    
    // for integral RepeatedField
    template <typename T>
    size_t Deserialize(::google::protobuf::RepeatedField<T> *out,
                       std::enable_if_t<std::is_integral<T>::value>* = 0);
    
    // default to calling member function
    template <typename T>
    size_t Deserialize(T *obj, std::enable_if_t<std::is_class<T>::value>* = 0) 
    {
        obj->Deserialize(stream_);
    }
    
    // deserialize variable length integer
    uint64_t SerReadVarInt();
    
    // Lowest-level deserialization and conversion.
    template <typename T> size_t SerReadData(T*);
};

template <typename Stream>
size_t Deserializer<Stream>::Deserialize(double *out) 
{
    uint64_t i;
    size_t size = SerReadData(&i);
    *out = BinaryToDouble(i);  
    
    return size;
}

template <typename Stream>
size_t Deserializer<Stream>::Deserialize(float *out) 
{
    uint32_t i;
    size_t size = SerReadData(&i);
    *out = BinaryToFloat(i);
    
    return size;
}

template <typename Stream>
size_t Deserializer<Stream>::Deserialize(std::string *out)
{
    size_t size = SerReadVarInt();
    char c;
    
    // Read all size characters, pushing all non-null (may be many).
    out->clear();
    out->reserve(size);
    for (size_t i = 0; i < size; i++) {
        stream_.read(&c, 1);
        if (c == '\0') {
            break;
        }
        out->push_back(c);
    }
    
    return size + 1;
}

template <typename Stream>
template <typename T, size_t N>
size_t Deserializer<Stream>::Deserialize(std::array<T, N> *out,
                                         std::enable_if_t<std::is_arithmetic<T>::value>*)
{
    size_t size = 0;
    
    for (auto it = out->begin(); it != out->end(); it++) {
        size += Deserialize(&(*it));
    }
    
    return size;
}

template <typename Stream>
template <typename T>
size_t Deserializer<Stream>::Deserialize(std::vector<T> *out,
                                         std::enable_if_t<std::is_arithmetic<T>::value>*)
{
    // Limit size per read so bogus size value won't cause out of memory
    size_t size = 0;
    uint64_t count = SerReadVarInt();
    if (count*sizeof(T) > kMaxBlockSize) {
        BTCLOG(LOG_LEVEL_ERROR) << "vector size larger than max block size";
        SingletonInterruptor::GetInstance().Interrupt();
    }
    size += 1;
    
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); ++it) {
        size += Deserialize(&(*it));
    }
    
    return size;
}

template <typename Stream>
size_t Deserializer<Stream>::Deserialize(std::vector<std::string> *out)
{
    uint64_t count = SerReadVarInt();
    size_t size = 1;   
    
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); ++it) {
        Deserialize(&(*it));
        size += it->size();
        if (size > kMaxBlockSize) {
            BTCLOG(LOG_LEVEL_ERROR) << "vector size larger than max block size";
            SingletonInterruptor::GetInstance().Interrupt();
        }
    }
    
    return size;
}

template <typename Stream>
template <typename T> 
size_t Deserializer<Stream>::Deserialize(std::vector<T> *out,
                                         std::enable_if_t<std::is_class<T>::value>*)
{
    uint64_t count = SerReadVarInt();
    size_t size = 1;   
    
    out->clear();
    out->resize(count);
    for (auto it = out->begin(); it != out->end(); ++it) {
        size += Deserialize(&(*it));
        if (size > kMaxBlockSize) {
            BTCLOG(LOG_LEVEL_ERROR) << "vector size larger than max block size";
            SingletonInterruptor::GetInstance().Interrupt();
        }
    }
    
    return size;
}

template <typename Stream>
template <typename T>
size_t Deserializer<Stream>::Deserialize(::google::protobuf::RepeatedField<T> *out,
                                         std::enable_if_t<std::is_integral<T>::value>*)
{
    size_t size = 0;
    
    for (uint8_t *p = reinterpret_cast<uint8_t*>(out->begin()); 
            p != reinterpret_cast<uint8_t*>(out->end()); p++) {
        size += Deserialize(p);
    }
    
    return size;
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
        if (varint < kVarint16bits) {
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
        }
    }
    else if (count == kVarint32bits) {
        SerReadData(reinterpret_cast<uint32_t*>(&varint));
        if (varint <= std::numeric_limits<uint16_t>::max()) {
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
        }
    }
    else {
        SerReadData(&varint);
        if (varint <= std::numeric_limits<uint32_t>::max()) {
            throw std::ios_base::failure("non-canonical SerReadVarInt()");
        }
    }
    if (varint > kMaxVardataSize) {
        throw std::ios_base::failure("SerReadVarInt(): size too large");
    }
    
    return varint;
}

template <typename Stream>
template <typename T>
size_t Deserializer<Stream>::SerReadData(T *obj)
{
    uint8_t data[sizeof(T)];
    size_t size = stream_.read(reinterpret_cast<char*>(&data[0]), sizeof data);
    *obj = FromLittleEndian<T>(data);
    
    return size;
}

} // namespace util
} // namespace btclite

#endif // BTCLITE_SERIALIZE_H
