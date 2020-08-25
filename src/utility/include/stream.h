#ifndef BTCLITE_STREAM_H
#define BTCLITE_STREAM_H

#include "blob.h"
#include "serialize.h"
#include "util.h"


namespace btclite {
namespace util {

// memory input/output stream
class MemoryStream : Uncopyable {
public:
    using Container = std::vector<uint8_t>;
    using ByteSinkType = ByteSink<Container>;
    using ByteSourceType = ByteSource<Container>;
    
    MemoryStream()
        : vec_(), byte_sink_(vec_), byte_source_(vec_) {}
    
    template <typename T>
    MemoryStream& operator<<(const T& obj)
    {
        Serializer<ByteSinkType> serializer(byte_sink_);
        serializer.SerialWrite(obj);
        return *this;
    }
    
    template <typename T>
    MemoryStream& operator>>(T& obj)
    {
        util::Deserializer<ByteSourceType> deserializer(byte_source_);
        deserializer.SerialRead(&obj);
        return *this;
    }
    
    uint8_t *Data()
    {
        return vec_.data();
    }
    
    void Clear()
    {
        vec_.clear();
    }
    
    size_t Size()
    {
        return vec_.size();
    }
    
private:
    Container vec_;
    ByteSinkType byte_sink_;
    ByteSourceType byte_source_;
};

} // namespace util
} // namespace btclite


#endif // BTCLITE_STREAM_H
