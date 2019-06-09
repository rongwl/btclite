#ifndef BTCLITE_UTILTIME_H
#define BTCLITE_UTILTIME_H

#include <chrono>
#include <cassert>
#include <sstream>


int64_t GetTimeSeconds();
int64_t GetTimeMicros();
std::string DateTimeStrFormat();

#endif // BTCLITE_UTILTIME_H
