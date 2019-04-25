#ifndef BTCLITE_UTIL_TESTS_H
#define BTCLITE_UTIL_TESTS_H

#include "util.h"

class TestArgs : public Args {
public:
    bool Init(int argc, const char* const argv[]) {}
    void Parse(int argc, const char* const argv[]) {}
    bool InitParameters() {}
    
    
};

#endif // BTCLITE_UTIL_TESTS_H
