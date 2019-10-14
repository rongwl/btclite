#ifndef BTCLITE_UTILTIME_H
#define BTCLITE_UTILTIME_H


#include "util.h"


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

namespace btclite {
namespace utility {
namespace util_time {

int64_t GetTimeSeconds();
int64_t GetTimeMillis();
int64_t GetTimeMicros();
int64_t GetAdjustedTime();
void AddTimeData(const std::string& ip, int64_t offset_sample);

} // namespace time
} // namespace util
} // namespace btclite


#endif // BTCLITE_UTILTIME_H
