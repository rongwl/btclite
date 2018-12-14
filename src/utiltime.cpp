#if defined(HAVE_CONFIG_H)
#include "config/btcdemo-config.h"
#endif

#include "utilstrencodings.h"
#include "utiltime.h"

#include <boost/date_time/posix_time/posix_time.hpp>

int64_t GetTimeMicros()
{
    int64_t now = (boost::posix_time::microsec_clock::universal_time() -
                   boost::posix_time::ptime(boost::gregorian::date(1970,1,1))).total_microseconds();
    assert(now > 0);
    return now;
}

std::string DateTimeStrFormat()
{
	boost::posix_time::ptime t = boost::posix_time::microsec_clock::universal_time();	
    return boost::posix_time::to_simple_string(t);
}
