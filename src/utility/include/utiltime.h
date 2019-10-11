#ifndef BTCLITE_UTILTIME_H
#define BTCLITE_UTILTIME_H


#include <cassert>
#include <iomanip>
#include <sys/time.h>

#include "util.h"


class Time {
public:
    static int64_t GetTimeSeconds()
    {
        time_t now = time(nullptr);
        assert(now > 0);
        return now;
    }
    
    static int64_t GetTimeMillis()
    {
        struct timeval tv;
        assert(0 == gettimeofday(&tv, NULL));
        return tv.tv_sec*1000 + tv.tv_usec/1000;
    }
    
    static int64_t GetTimeMicros()
    {
        struct timeval tv;
        assert(0 == gettimeofday(&tv, NULL));
        return tv.tv_sec*1000000+tv.tv_usec;
    }
    
    int64_t GetAdjustedTime()
    {
        LOCK(cs_time_offset_);
        return GetTimeSeconds() + time_offset_;
    }
    
    void AddTimeData(const std::string& ip, int64_t offset_sample);
    
private:
    CriticalSection cs_time_offset_;
    int64_t time_offset_;
};

class SingletonTime : Uncopyable {
public:
    static Time& GetInstance()
    {
        static Time time;
        return time;
    }
    
private:
    SingletonTime() {}
};

#endif // BTCLITE_UTILTIME_H
