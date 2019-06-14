#ifndef BTCLITE_FULLNODE_CONFIG_H
#define BTCLITE_FULLNODE_CONFIG_H


#include "environment.h"
#include "util.h"


#define FULLNODE_OPTION_CONNECT  "connect"
#define FULLNODE_OPTION_LISTEN   "listen"
#define FULLNODE_OPTION_DISCOVER "discover"
#define FULLNODE_OPTION_DNSSEED  "dnsseed"

#define DEFAULT_DATA_DIR        ".btc-fullnode"
#define DEFAULT_CONFIG_FILE     "btc-fullnode.conf"

#define DEFAULT_LISTEN    "1"
#define DEFAULT_DISCOVER  "1"
#define DEFAULT_DNSSEED   "1"


class FullNodeArgs : public Args {
public:    
    bool Init(int argc, const char* const argv[]);
    bool InitParameters();
    void PrintUsage() const;

private:
    const std::string bin_name_ = "btc-fullnode";
    
    void Parse(int argc, const char* const argv[]);
    void CheckArguments() const;
};

class FullNodeDataFiles : public DataFiles {
public:
    FullNodeDataFiles()
        : DataFiles(DefaultDataDirPath(), DEFAULT_CONFIG_FILE) {}
    
    bool Init(const std::string& path = DefaultDataDirPath(),
              const std::string& config_file = DEFAULT_CONFIG_FILE);
    
    static fs::path DefaultDataDirPath()
    {
        return PathHome() / DEFAULT_DATA_DIR;
    }
};

class FullNodeConfig : public ExecutorConfig {
public:
    FullNodeConfig(int argc, const char* const argv[])
        : ExecutorConfig(argc, argv) {}
    
    bool Init();
    
    const Args& args() const
    {
        return args_;
    }
    
    const DataFiles& data_files() const
    {
        return data_files_;
    }
    
private:
    FullNodeArgs args_;
    FullNodeDataFiles data_files_;
    
    bool InitDataFiles();
    bool LoadConfigFile();
};


#endif // BTCLITE_FULLNODE_CONFIG_H
