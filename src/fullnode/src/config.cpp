#include <algorithm>
#include <arpa/inet.h>

#include "fullnode/include/config.h"
#include "error.h"
#include "utility/include/logging.h"


void FullNodeConfig::Parse(int argc, const char* const argv[])
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
        { FULLNODE_OPTION_LISTEN,   required_argument,  NULL,  0  },
        { FULLNODE_OPTION_DISCOVER, required_argument,  NULL,  0, },
        { FULLNODE_OPTION_DNSSEED,  required_argument,  NULL,  0, },
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
                        throw Exception(ErrorCode::invalid_option, "option '--" + str + "' requires an argument");
                    }
                }

                args_.SetArgs(str, str_val);
                break;
            }
            case 'h' :
            case '?' :
                throw Exception(ErrorCode::show_help, "");
                break;
            default :
                break;
        }
        option_index = 0;
    }
    
    CheckArguments();
}

void FullNodeConfig::CheckArguments() const
{
    ExecutorConfig::CheckArguments();
    
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
            throw Exception(ErrorCode::invalid_argument, "invalid ip '" + *result + "'");
    }
    
    // --listen
    if (args_.IsArgSet(FULLNODE_OPTION_LISTEN))
    {
        std::string val = args_.GetArg(FULLNODE_OPTION_LISTEN, DEFAULT_LISTEN);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }

    // --discover
    if (args_.IsArgSet(FULLNODE_OPTION_DISCOVER))
    {
        std::string val = args_.GetArg(FULLNODE_OPTION_DISCOVER, DEFAULT_DISCOVER);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }
    
    // --dnsseed
    if (args_.IsArgSet(FULLNODE_OPTION_DNSSEED))
    {
        std::string val = args_.GetArg(FULLNODE_OPTION_DNSSEED, DEFAULT_DNSSEED);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }
}

void FullNodeHelpInfo::PrintUsage()
{
    fprintf(stdout, "Usage: %s [OPTIONS...]\n\n", FULLNODE_BIN_NAME);
    HelpInfo::PrintUsage();
    fprintf(stdout, "  --datadir=<dir>       specify data directory.\n");
    fprintf(stdout, "  --conf=<file>         specify configuration file (default: %s)\n", DEFAULT_CONFIG_FILE);
    fprintf(stdout, "\nConnection Options:\n");
    fprintf(stdout, "  --connect=<ip>        connect only to the specified node(s); -connect=0 \n");
    fprintf(stdout, "                        disables automatic connections\n");
    fprintf(stdout, "  --listen=<1/0>        accept connections from outside (default: %s)\n", DEFAULT_LISTEN);
    fprintf(stdout, "  --discover=<1/0>      discover own ip address (default: %s)\n", DEFAULT_DISCOVER);
    fprintf(stdout, "  --dnsseed=<1/0>       query for peer addresses via DNS lookup (default: %s)\n", DEFAULT_DNSSEED);
    //              "                                                                                "

}

bool FullNodeConfig::InitDataDir()
{
    path_data_dir_ = path_default_data_path_;
    if (args_.IsArgSet(GLOBAL_OPTION_DATADIR)) {
        const std::string path = args_.GetArg(GLOBAL_OPTION_DATADIR, DEFAULT_DATA_DIR);
        if (fs::is_directory(path))
            path_data_dir_ = fs::path(path);
        else
            BTCLOG(LOG_LEVEL_WARNING) << "Specified data path \"" << path << "\" does not exist. Use default data path.";
    }
    
    if (path_data_dir_ == path_default_data_path_)
        fs::create_directories(path_data_dir_); // create default data dir if it does not exist
    
    config_file_ = DEFAULT_CONFIG_FILE;
    if (args_.IsArgSet(GLOBAL_OPTION_CONF)) {
        config_file_ = args_.GetArg(GLOBAL_OPTION_CONF, DEFAULT_CONFIG_FILE);
        if (!std::ifstream(path_data_dir() / config_file_).good())
            BTCLOG(LOG_LEVEL_WARNING) << "Specified config file \"" << path_data_dir().c_str() << "/" << config_file_
                                      << "\" does not exist. Use default config file.";
    }
    
    if (config_file_ == DEFAULT_CONFIG_FILE)
        std::ofstream file(path_data_dir_ / config_file_); // create default config file if it does not exist    
    
    return ParseFromFile(path_data_dir_ / config_file_);
}

bool FullNodeConfig::InitParameters()
{
    if (!ExecutorConfig::InitParameters())
        return false;
    
    // --connect
    if (args_.IsArgSet(FULLNODE_OPTION_CONNECT)) {
        args_.SetArg(FULLNODE_OPTION_LISTEN, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --connect=1 -> set --listen=0";
        args_.SetArg(FULLNODE_OPTION_DNSSEED, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --connect=1 -> set --dnsseed=0";
    }
    
    // --listen
    if (args_.GetArg(FULLNODE_OPTION_LISTEN, DEFAULT_LISTEN) == "0") {
        args_.SetArg(FULLNODE_OPTION_DISCOVER, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --listen=0 -> set --discover=0";
    }
    
    return true;
}
