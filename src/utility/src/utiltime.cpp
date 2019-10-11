#include "utiltime.h"

#include <cstdlib>
#include <set>

#include "constants.h"
#include "util.h"


void Time::AddTimeData(const std::string& ip, int64_t offset_sample)
{
    // Ignore duplicates
    static std::set<std::string> set_known;
    if (set_known.size() == kMaxTimedataSamples)
        return;
    if (!set_known.insert(ip).second)
        return;

    // Add data
    static MedianFilter<int64_t> time_offsets(kMaxTimedataSamples, 0);
    time_offsets.Input(offset_sample);
    BTCLOG(LOG_LEVEL_VERBOSE) << "Added time data, samples " << time_offsets.Size() << ", offset " << offset_sample;
    
    if (time_offsets.Size() >= 5 && time_offsets.Size() % 2 == 1)
    {
        int64_t median = time_offsets.Median();
        LOCK(cs_time_offset_);
        // Only let other nodes change our time by so much
        if (abs(median) <= 70*60)
            time_offset_ = median;
        else
            time_offset_ = 0;
        
        BTCLOG(LOG_LEVEL_VERBOSE) << "Set new btc time offset:" << time_offset_;
    }    
}
