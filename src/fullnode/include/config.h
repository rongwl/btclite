#ifndef BTCLITE_FULLNODE_CONFIG_H
#define BTCLITE_FULLNODE_CONFIG_H


#include "btcnet.h"
#include "util.h"


namespace btclite {
namespace fullnode {

#define FULLNODE_BIN_NAME        "btc-fullnode"

#define FULLNODE_OPTION_CONNECT  "connect"

#define DEFAULT_DATA_DIR        ".btc-fullnode"
#define DEFAULT_CONFIG_FILE     "btc-fullnode.conf"

#define DEFAULT_LISTEN    "1"
#define DEFAULT_DISCOVER  "1"
#define DEFAULT_DNSSEED   "1"


class FullNodeConfig final : public btclite::util::Configuration {
public:    
    void ParseParameters(int argc, const char* const argv[]);
    bool InitDataDir();
    
private:
    bool InitArgsCustomized();
    void CheckArgsCustomized() const;
    void PrintUsageCustomized() const;
};

} // namespace fullnode
} // namespace btclite

#endif // BTCLITE_FULLNODE_CONFIG_H
