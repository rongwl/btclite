#include <algorithm>
#include <arpa/inet.h>

#include "fullnode/include/config.h"
#include "error.h"
#include "utility/include/logging.h"


bool FullNodeArgs::Init(int argc, const char* const argv[])
{
    Parse(argc, argv);
    
    return true;
}

void FullNodeArgs::Parse(int argc, const char* const argv[])
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

    Clear();

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

                SetArgs(str, str_val);
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

void FullNodeArgs::CheckArguments() const
{
    Args::CheckArguments();
    
    // --connect
    if (IsArgSet(FULLNODE_OPTION_CONNECT)) {
        const std::vector<std::string> arg_values = GetArgs(FULLNODE_OPTION_CONNECT);
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
    if (IsArgSet(FULLNODE_OPTION_LISTEN))
    {
        std::string val = GetArg(FULLNODE_OPTION_LISTEN, DEFAULT_LISTEN);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }

    // --discover
    if (IsArgSet(FULLNODE_OPTION_DISCOVER))
    {
        std::string val = GetArg(FULLNODE_OPTION_DISCOVER, DEFAULT_DISCOVER);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }
    
    // --dnsseed
    if (IsArgSet(FULLNODE_OPTION_DNSSEED))
    {
        std::string val = GetArg(FULLNODE_OPTION_DNSSEED, DEFAULT_DNSSEED);
        if (val != "1" && val != "0")
            throw Exception(ErrorCode::invalid_argument, "invalid argument '" + val + "'");
    }
}

void FullNodeArgs::PrintUsage() const
{
    fprintf(stdout, "Usage: %s [OPTIONS...]\n\n", bin_name_.c_str());
    Args::PrintUsage();
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

bool FullNodeArgs::InitParameters()
{
    if (!Args::InitParameters())
        return false;
    
    // --connect
    if (IsArgSet(FULLNODE_OPTION_CONNECT)) {
        SetArg(FULLNODE_OPTION_LISTEN, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --connect=1 -> set --listen=0";
        SetArg(FULLNODE_OPTION_DNSSEED, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --connect=1 -> set --dnsseed=0";
    }
    
    // --listen
    if (GetArg(FULLNODE_OPTION_LISTEN, DEFAULT_LISTEN) == "0") {
        SetArg(FULLNODE_OPTION_DISCOVER, "0");
        BTCLOG(LOG_LEVEL_DEBUG) << "set --listen=0 -> set --discover=0";
    }
    
    return true;
}

bool FullNodeDataFiles::Init(const std::string& dir_path, const std::string& config_file)
{
    if (!fs::is_directory(dir_path)) {
        BTCLOG(LOG_LEVEL_WARNING) << "Specified data path \"" << data_dir().c_str()
                                 << "\" does not exist. Use default data path.";
        if (dir_path == DefaultDataDirPath())
            fs::create_directories(dir_path);        
    }
    else
        set_data_dir(fs::path(dir_path));
    
    std::ifstream ifs(data_dir() / config_file);
    if (!ifs.good()) {
        BTCLOG(LOG_LEVEL_WARNING) << "Specified config file \"" << data_dir().c_str() << "/" << config_file
                                 << "\" does not exist. Use default config file.";
        if (config_file == DEFAULT_CONFIG_FILE)
            std::ofstream file(DataFiles::config_file()); // create default config file if it does not exist
    }
    else
        set_config_file(config_file);
    
    return true;
}

fs::path FullNodeDataFiles::DefaultDataDirPath()
{
    char *home_path = getenv("HOME");            
    if (home_path == NULL)
        return fs::path("/") / DEFAULT_DATA_DIR;
    else
        return fs::path(home_path) / DEFAULT_DATA_DIR;
}
