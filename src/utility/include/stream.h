#ifndef BTCLITE_STREAM_H
#define BTCLITE_STREAM_H


#include "serialize.h"


/* 
 * Minimal stream for overwriting and/or appending to an existing byte vector
 * The referenced vector will grow as necessary
 */
class VecWStream {
public:
    using Container = std::vector<uint8_t>;
    using ByteSinkType = ByteSink<Container>;
    
    VecWStream()
        : vec_(), byte_sink_(vec_) {}    
    
    template <typename T>
    VecWStream& operator<<(const T& obj)
    {
        Serializer<ByteSinkType> serial(byte_sink_);
        serial.SerialWrite(obj);
        return *this;
    }
    
    void Clear()
    {
        vec_.clear();
    }
    
private:
    Container vec_;
    ByteSinkType byte_sink_;
};

#endif // BTCLITE_STREAM_H
