#ifndef BTCLITE_STREAM_H
#define BTCLITE_STREAM_H


#include "serialize.h"


/* 
 * Minimal stream for overwriting and/or appending to an existing byte vector
 * The referenced vector will grow as necessary
 */
class MemOstream {
public:
    using Container = std::vector<uint8_t>;
    using ByteSinkType = ByteSink<Container>;
    
    MemOstream()
        : vec_(), byte_sink_(vec_) {}    
    
    template <typename T>
    MemOstream& operator<<(const T& obj)
    {
        Serializer<ByteSinkType> serial(byte_sink_);
        serial.SerialWrite(obj);
        return *this;
    }
    
    const Container& vec() const
    {
        return vec_;
    }
    
    void Clear()
    {
        vec_.clear();
    }
    
private:
    Container vec_;
    ByteSinkType byte_sink_;
};

// memory input stream
template <typename ByteSource>
class MemIstream {
public:    
    MemIstream(ByteSource& source)
        : byte_source_(source) {}
    
    template <typename T>
    MemIstream& operator>>(T& obj)
    {
        Serializer<ByteSource> serial(byte_source_);
        serial.SerialRead(&obj);
        return *this;
    }
    
private:
    ByteSource& byte_source_;
};

#endif // BTCLITE_STREAM_H
