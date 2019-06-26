#ifndef BTCLITE_FULLNODE_CONFIG_H
#define BTCLITE_FULLNODE_CONFIG_H


#include "environment.h"
#include "util.h"


#define FULLNODE_BIN_NAME        "btc-fullnode"

#define FULLNODE_OPTION_CONNECT  "connect"
#define FULLNODE_OPTION_LISTEN   "listen"
#define FULLNODE_OPTION_DISCOVER "discover"
#define FULLNODE_OPTION_DNSSEED  "dnsseed"

#define DEFAULT_DATA_DIR        ".btc-fullnode"
#define DEFAULT_CONFIG_FILE     "btc-fullnode.conf"

#define DEFAULT_LISTEN    "1"
#define DEFAULT_DISCOVER  "1"
#define DEFAULT_DNSSEED   "1"


class FullNodeConfig : public ExecutorConfig {
public:
    FullNodeConfig(int argc, const char* const argv[])
        : path_default_data_path_(PathHome() / DEFAULT_DATA_DIR)
    {
        Parse(argc, argv);
    }
    
    bool InitDataDir();
    bool InitParameters();
    
private:
    fs::path path_default_data_path_;
    
    void Parse(int argc, const char* const argv[]);
    void CheckArguments() const;
};

class FullNodeHelpInfo {
public:
    static void PrintUsage();
};


#endif // BTCLITE_FULLNODE_CONFIG_H
