#ifndef BTCLITE_UTIL_TESTS_H
#define BTCLITE_UTIL_TESTS_H

#include "util.h"


class TestExecutorConfig : public ExecutorConfig {
public:
    void Prase(int argc, const char* const argv[]) {}
    bool InitDataDir() {}
    
    static void set_path_data_dir(const fs::path& path)
    {
        path_data_dir_ = path;
    }
};

#endif // BTCLITE_UTIL_TESTS_H
