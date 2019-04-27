#ifndef BTCLITE_UTIL_TESTS_H
#define BTCLITE_UTIL_TESTS_H

#include "util.h"

class TestArgs : public Args {
public:
    bool Init(int argc, const char* const argv[]) {}
    void Parse(int argc, const char* const argv[]) {}
    bool InitParameters() {}    
};


class TestDataFiles : public DataFiles {
public:
    using DataFiles::DataFiles;
    bool Init(const std::string& path, const std::string& config_file) {}
};

#endif // BTCLITE_UTIL_TESTS_H
