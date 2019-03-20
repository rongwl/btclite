#include "executor.h"

bool FullNodeArgs::Init(int argc, char* const argv[])
{
	if (!Parse(argc, argv))
		return false;
	if (!InitLogging(argv[0]))
		return false;
	if (!InitParameters())
		return false;
	
	return true;
}

bool FullNodeArgs::Parse(int argc, char* const argv[])
{
	if (!CheckOptions(argc, argv)) {
		PrintUsage();
		return false;
	}

	const static struct option fullnode_options[] {
		{ GLOBAL_OPTION_HELP,       no_argument,        NULL, 'h' },
		{ GLOBAL_OPTION_DEBUG,      required_argument,  NULL,  0  },
		{ GLOBAL_OPTION_LOGLEVEL,   required_argument,  NULL,  0  },
		{ GLOBAL_OPTION_DATADIR,    required_argument,  NULL,  0  },
		{ GLOBAL_OPTION_CONF,       required_argument,  NULL,  0  },
		{ FULLNODE_OPTION_CONNECT,  required_argument,  NULL,  0  },
	    { FULLNODE_OPTION_LISTEN,   required_argument,  NULL,  0  },
		{ FULLNODE_OPTION_DISCOVER, required_argument,  NULL,  0, },
		{ FULLNODE_OPTION_DNSSEED,  required_argument,  NULL,  0, },
		{ 0,                        0,                  0,     0  }
	};
	int c, option_index;
	
	LOCK(cs_args_);
	map_args_.clear();
	map_multi_args_.clear();
	
	while ((c = getopt_long(argc, argv, "h?", fullnode_options, &option_index)) != -1) {
		switch (c) {
			case 0 : 
			{
				std::string str(fullnode_options[option_index].name);
				std::string str_val(optarg);
				if (str_val.empty()) {
					fprintf(stderr, "%s: option '--%s' requires an argument\n", argv[0], str.c_str());
					PrintUsage();
					return false;
				}

				map_args_[str] = str_val;
				map_multi_args_[str].push_back(str_val);
				break;
			}
			case 'h' :
			case '?' :
				PrintUsage(); 
				return false;
			default :
				break;
		}
	}
	
	return true;
}

void FullNodeArgs::PrintUsage()
{
	ArgsManager::PrintUsage();
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
	if (!ArgsManager::InitParameters())
		return false;
	
	// --connect
	if (IsArgSet(FULLNODE_OPTION_CONNECT)) {
		SetArg(FULLNODE_OPTION_LISTEN, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --connect=1 -> set --listen=0";
		SetArg(FULLNODE_OPTION_DNSSEED, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --connect=1 -> set --dnsseed=0";
	}
	
	// --listen
	if (GetArg(FULLNODE_OPTION_LISTEN, DEFAULT_LISTEN) == "0") {
		SetArg(FULLNODE_OPTION_DISCOVER, "0");
		BTCLOG(LOGLEVEL_DEBUG) << "set --listen=0 -> set --discover=0";
	}

	return true;
}

FullNodeDataFiles::FullNodeDataFiles(const std::string& path) : DataFilesManager(path)
{
	LOCK(cs_path_);
	
	if (!fs::is_directory(data_dir_)) {
		BTCLOG(LOGLEVEL_WARNING) << "Specified data path \"" << data_dir_.c_str()
								 << "\" does not exist. Use default data path.";
		
		char *home_path = getenv("HOME");			
		if (home_path == NULL)
			data_dir_ = fs::path("/") / DEFAULT_DATA_DIR;
		else
			data_dir_ = fs::path(home_path) / DEFAULT_DATA_DIR;
		
		fs::create_directories(data_dir_);
	}	
}

bool FullNodeMain::Init()
{
	if (!CheckDataPath())
		return false;

	if (!InitConfigFile())
		return false;

	if (!BasicSetup())
		return false;

	return true;
}

bool FullNodeMain::Start()
{
	return true;
}

bool FullNodeMain::Run()
{
	return true;
}

void FullNodeMain::Interrupt()
{
	
}

void FullNodeMain::Stop()
{
	BTCLOG(LOGLEVEL_INFO) << __func__ << ": progress...";
	
	BTCLOG(LOGLEVEL_INFO) << __func__ << ": done";
}

bool FullNodeMain::CheckDataPath()
{
	if (!fs::is_directory(data_files_.DataDir())) {
		BTCLOG(LOGLEVEL_ERROR) << "Error: Specified data directory \"" << data_files_.DataDir().c_str() << "\" does not exist.";
		return false;
	}
	
	return true;
}

bool FullNodeMain::InitConfigFile()
{
	data_files_.set_configFile(args_.GetArg(GLOBAL_OPTION_CONF, DEFAULT_CONFIG_FILE));
	args_.ParseFromFile(data_files_.ConfigFile().c_str());
	
	return true;
}


