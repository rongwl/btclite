#include "fullnode/include/config.h"

#include <algorithm>
#include <arpa/inet.h>

#include "error.h"
#include "utility/include/logging.h"


namespace btclite {
namespace fullnode {

using namespace btclite::util;

void FullNodeConfig::ParseParameters(int argc, const char* const argv[])
{
    CheckOptions(argc, argv);

    const static struct option fullnode_options[] {
        { GLOBAL_OPTION_HELP,       no_argument,        NULL, 'h' },
        { GLOBAL_OPTION_DEBUG,      required_argument,  NULL,  0  },
        { GLOBAL_OPTION_LOGLEVEL,   required_argument,  NULL,  0  },
        { GLOBAL_OPTION_DATADIR,    required_argument,  NULL,  0  },
        { GLOBAL_OPTION_CONF,       required_argument,  NULL,  0  },
        { GLOBAL_OPTION_TESTNET,    no_argument,        NULL,  0  },
        { GLOBAL_OPTION_REGTEST,    no_argument,        NULL,  0  },
        { FULLNODE_OPTION_CONNECT,  required_argument,  NULL,  0  },
        { 0,                        0,                  0,     0  }
    };
    int c, option_index = 0;

    args_.Clear();

    while ((c = getopt_long(argc, const_cast<char* const*>(argv), "h?", fullnode_options, &option_index)) != -1) {
        switch (c) {
            case 0 : 
            {
                std::string str(fullnode_options[option_index].name);
                std::string str_val;
                if (fullnode_options[option_index].has_arg) {
                    str_val = std::string(optarg);
                    if (str_val.empty()) {
                        throw Exception(ErrorCode::kInvalidOption, "option '--" + str + "' requires an argument");
                    }
                }

                args_.SetArgs(str, str_val);
                break;
            }
            case 'h' :
            case '?' :
                throw Exception(ErrorCode::kShowHelp, "");
                break;
            default :
                break;
        }
        option_index = 0;
    }
    
    CheckArgs();
}

void FullNodeConfig::PrintUsageCustomized() const
{
    fprintf(stdout, "  --debug=<module>      output debugging information(default: 0)\n");
    fprintf(stdout, "                        <module> can be 1(output all debugging information),\n");
    fprintf(stdout, "                        mempool, net\n");
    fprintf(stdout, "  --loglevel=<level>    specify the type of message being printed(default: %s)\n", DEFAULT_LOG_LEVEL);
    fprintf(stdout, "                        <level> can be:\n");
    fprintf(stdout, "                            0(A fatal condition),\n");
    fprintf(stdout, "                            1(An error has occurred),\n");
    fprintf(stdout, "                            2(A warning),\n");
    fprintf(stdout, "                            3(Normal message),\n");
    fprintf(stdout, "                            4(Debug information),\n");
    fprintf(stdout, "                            5(Verbose information\n");
    fprintf(stdout, "  --datadir=<dir>       specify data directory.\n");
    fprintf(stdout, "  --conf=<file>         specify configuration file (default: %s)\n", DEFAULT_CONFIG_FILE);
    fprintf(stdout, "\n");
    fprintf(stdout, "Chain Selection Options:\n");
    fprintf(stdout, "  --testnet             Use the test chain\n");
    fprintf(stdout, "  --regtest             Enter regression test mode, which uses a special chain\n");
    fprintf(stdout, "                        in which blocks can be solved instantly. This is intended\n");
    fprintf(stdout, "                        for regression testing tools and app development.\n");
    fprintf(stdout, "\nConnection Options:\n");
    fprintf(stdout, "  --connect=<ip>        connect only to the specified node(s); -connect=0 \n");
    fprintf(stdout, "                        disables automatic connections\n");
    //              "                                                                                "

}

bool FullNodeConfig::InitDataDir()
{
    const fs::path path_default_data_dir_ = PathHome() / DEFAULT_DATA_DIR;
    std::string dir_name = "mainnet";

    if (btcnet_ == BtcNet::kTestNet) {
        dir_name = "testnet";
    } else if (btcnet_ == BtcNet::kRegTest) {
        dir_name = "regtest";
    }
    path_data_dir_ = path_default_data_dir_ / dir_name;
    
    if (args_.IsArgSet(GLOBAL_OPTION_DATADIR)) {
        const std::string path = args_.GetArg(GLOBAL_OPTION_DATADIR, DEFAULT_DATA_DIR);
        if (fs::is_directory(path)) {
            path_data_dir_ = fs::path(path) / dir_name;
        } else {
            BTCLOG(LOG_LEVEL_WARNING) << "Specified data path \"" << path 
                                      << "\" does not exist. Use default data path.";
        }
    }

    if (path_data_dir_ == path_default_data_dir_ / dir_name) {
        // create default data dir if it does not exist
        fs::create_directories(path_data_dir_); 
    }
    
    config_file_ = DEFAULT_CONFIG_FILE;
    if (args_.IsArgSet(GLOBAL_OPTION_CONF)) {
        config_file_ = args_.GetArg(GLOBAL_OPTION_CONF, DEFAULT_CONFIG_FILE);
        if (!std::ifstream(path_data_dir() / config_file_).good()) {
            BTCLOG(LOG_LEVEL_WARNING) << "Specified config file \"" << path_data_dir().c_str() 
                                      << "/" << config_file_
                                      << "\" does not exist. Use default config file.";
        }
    }
    
    if (config_file_ == DEFAULT_CONFIG_FILE) {
        // create default config file if it does not exist    
        std::ofstream file(path_data_dir_ / config_file_);
    }
    
    return ParseFromFile(path_data_dir_ / config_file_);
}

bool FullNodeConfig::InitArgsCustomized()
{    
    return true;
}

void FullNodeConfig::CheckArgsCustomized() const
{
    // --connect
    if (args_.IsArgSet(FULLNODE_OPTION_CONNECT)) {
        const std::vector<std::string> arg_values = args_.GetArgs(FULLNODE_OPTION_CONNECT);
        auto is_invalid_ip =  [](const std::string& val)
        {
            struct sockaddr_in sa;
            int result = inet_pton(AF_INET, val.c_str(), &(sa.sin_addr));
            return result == 0;
        };
        auto result = std::find_if(arg_values.begin(), arg_values.end(), is_invalid_ip);
        if (result != arg_values.end())
            throw Exception(ErrorCode::kInvalidArg, "invalid ip '" + *result + "'");
    }
}

} // namespace fullnode
} // namespace btclite
