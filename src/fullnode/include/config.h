#ifndef BTCLITE_FULLNODE_CONFIG_H
#define BTCLITE_FULLNODE_CONFIG_H

#include "environment.h"
#include "chain/include/params.h"
#include "network/include/params.h"
#include "util.h"


#define FULLNODE_OPTION_CONNECT  "connect"
#define FULLNODE_OPTION_LISTEN   "listen"
#define FULLNODE_OPTION_DISCOVER "discover"
#define FULLNODE_OPTION_DNSSEED  "dnsseed"

#define DEFAULT_CONFIG_FILE     "btc-fullnode.conf"
#define DEFAULT_DATA_DIR        ".btc-fullnode"

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
};

class FullNodeDataFiles : public DataFiles {
public:
    using DataFiles::DataFiles;
    
    bool Init(const std::string& path, const std::string& config_file);
    static fs::path DefaultDataDirPath();
    
};


#endif // BTCLITE_FULLNODE_CONFIG_H
