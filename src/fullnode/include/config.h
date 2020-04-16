#ifndef BTCLITE_FULLNODE_CONFIG_H
#define BTCLITE_FULLNODE_CONFIG_H


#include "btcnet.h"
#include "util.h"


#define FULLNODE_BIN_NAME        "btc-fullnode"

#define FULLNODE_OPTION_CONNECT  "connect"

#define DEFAULT_DATA_DIR        ".btc-fullnode"
#define DEFAULT_CONFIG_FILE     "btc-fullnode.conf"

#define DEFAULT_LISTEN    "1"
#define DEFAULT_DISCOVER  "1"
#define DEFAULT_DNSSEED   "1"


class FullNodeConfig : public btclite::util::ExecutorConfig {
public:    
    void ParseParameters(int argc, const char* const argv[]);
    bool InitDataDir();
    bool InitParameters();
    
private:
    void CheckArgs() const;
};

class FullNodeHelpInfo {
public:
    static void PrintUsage();
};


#endif // BTCLITE_FULLNODE_CONFIG_H
