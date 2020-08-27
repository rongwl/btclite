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
    int64_t offset() const;    
    void set_offset(int64_t offset);

private:
    mutable CriticalSection cs_;
    int64_t offset_;
};


} // namespace util
} // namespace btclite


#endif // BTCLITE_UTIL_TIME_H
