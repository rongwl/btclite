#ifndef BTCLITE_UTIL_TIME_H
#define BTCLITE_UTIL_TIME_H


#include "network_address.h"
#include "util.h"


namespace btclite {
namespace util {

int64_t GetTimeSeconds();
int64_t GetTimeMillis();
int64_t GetTimeMicros();
int64_t GetAdjustedTime();
void AddTimeData(const network::NetAddr& addr, int64_t offset_sample);

class TimeOffset {
public:
    int64_t time_offset() const
    {
        LOCK(cs_time_offset_);
        return time_offset_;
    }
    
    void set_time_offset(int64_t offset)
    {
        LOCK(cs_time_offset_);
        time_offset_ = offset;
    }

private:
    mutable CriticalSection cs_time_offset_;
    int64_t time_offset_;
};

class SingletonTimeOffset : Uncopyable {
public:
    static TimeOffset& GetInstance() 
    {
        static TimeOffset time_offset;
        return time_offset;
    }
    
private:
    SingletonTimeOffset() {}
};

} // namespace util
} // namespace btclite


#endif // BTCLITE_UTIL_TIME_H
