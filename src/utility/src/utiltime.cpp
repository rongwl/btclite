#if defined(HAVE_CONFIG_H)
#include "config/btclite-config.h"
#endif

#include "string_encoding.h"
#include "utiltime.h"

#include <iomanip>
#include <sys/time.h>


int64_t GetTimeMicros()
{
    struct timeval tv;
    assert(0 == gettimeofday(&tv, NULL));
    return tv.tv_sec*1000000+tv.tv_usec;
}

std::string DateTimeStrFormat()
{
    struct timeval tv;
    assert(0 == gettimeofday(&tv, NULL));
    struct tm tm;
    assert(NULL != gmtime_r(&tv.tv_sec, &tm));
    std::stringstream ss;
    ss << tm.tm_year+1900 << "-" 
       << std::setw(2) << std::setfill('0') << tm.tm_mon+1 << "-"
       << std::setw(2) << std::setfill('0') << tm.tm_mday << " "
       << std::setw(2) << std::setfill('0') << tm.tm_hour << ":" 
       << std::setw(2) << std::setfill('0') << tm.tm_min << ":"
       << std::setw(2) << std::setfill('0') << tm.tm_sec << "."
       << tv.tv_usec;
    return ss.str();
}
