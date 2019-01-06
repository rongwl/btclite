#if defined(HAVE_CONFIG_H)
#include "config/btclite-config.h"
#endif

#include "init.h"
#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"

#include <getopt.h>
#include <locale>
#include <stdarg.h>

bool output_debug = false;
bool print_to_console = true;
bool print_to_file = false;

std::atomic<uint32_t> g_log_categories(0);
std::atomic<uint8_t> g_log_level(LogLevel::NOTICE); 

/**
 * started_newline is a state variable held by the calling context that will
 * suppress printing of the timestamp when multiple calls are made that don't
 * end in a newline. Initialize it to true, and hold it, in the calling context.
 */
static std::string LogTimestampStr(const std::string &str, std::atomic_bool *started_newline)
{
    std::string str_stamped;

    if (*started_newline)
        str_stamped = DateTimeStrFormat() + " " + str;
    else
        str_stamped = str;

    if (!str.empty() && str[str.size()-1] == '\n')
        *started_newline = true;
    else
        *started_newline = false;

    return str_stamped;
}

int LogPrintStr(const std::string &str)
{
    int ret = 0; // Returns total number of characters written
    static std::atomic_bool started_newline(true);

    std::string str_time_stamped = LogTimestampStr(str, &started_newline);

    ret = std::fwrite(str_time_stamped.data(), 1, str_time_stamped.size(), stdout);
    fflush(stdout);
    
    return ret;
}

/* Check options that getopt_long() can not print totally */
bool ArgsManager::CheckOptions(int argc, char* const argv[])
{
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		if ((str.length() > 2 && str.compare(0, 2, "--")) ||
		    !str.compare("--")) {
			fprintf(stdout, "%s: invalid option '%s'\n", argv[0], str.c_str());
			PrintUsage();
			return false;
		}
	}
	return true;
}

bool ArgsManager::ParseParameters(int argc, char* const argv[])
{
	if (!CheckOptions(argc, argv))
		return false;
	
	const static struct option options[] {
		{ BTCLITED_OPTION_HELP,     no_argument,        NULL, 'h' },
		{ BTCLITED_OPTION_DATADIR,  required_argument,  NULL,  0  },
		{ BTCLITED_OPTION_DEBUG,    required_argument,  NULL,  0  },
		{ BTCLITED_OPTION_CONF,     required_argument,  NULL,  0  },
		{ BTCLITED_OPTION_LOGLEVEL, required_argument,  NULL,  0  },
		{ BTCLITED_OPTION_CONNECT,  required_argument,  NULL,  0  },
	    { BTCLITED_OPTION_LISTEN,   required_argument,  NULL,  0  },
		{ BTCLITED_OPTION_DISCOVER, required_argument,  NULL,  0, },
		{ BTCLITED_OPTION_DNSSEED,  required_argument,  NULL,  0, },
		{ 0,                        0,                  0,     0  }
	};
	int c, option_index;
	
	LOCK(cs_args_);
	map_args_.clear();
	map_multi_args_.clear();
	
	while ((c = getopt_long(argc, argv, "h?", options, &option_index)) != -1) {
		switch (c) {
			case 0 : 
			{
				std::string str(options[option_index].name);
				std::string str_val(optarg);
				if (str_val.empty()) {
					fprintf(stderr, "%s: option '--%s' requires an argument\n", argv[0], str.c_str());
					PrintUsage();
					return false;
				}
				//LogPrint(LogLevel::DEBUG, "Parase option from command line: %s=%s\n", str.c_str(), str_val.c_str());
				map_args_[str] = str_val;
				map_multi_args_[str].push_back(str_val);
				break;
			}
			case 'h' :
			case '?' :
				PrintUsage();
				break;
			default :
				break;
		}
	}
	
	return true;
}

void ArgsManager::ReadConfigFile(const std::string& file_path) const
{
	fs::path path = g_path.GetConfigFile(file_path);
	std::ifstream ifs(path);
	if (!ifs.good()) {
		if (path == g_path.GetDataDir() / DEFAULT_CONFIG_FILE)
			std::ofstream file(path); // create default config file if it does not exist
		return; 
	}
	
	LOCK(cs_args_);
	std::string line;
	while (std::getline(ifs, line)) {
		line.erase(std::remove_if(line.begin(), line.end(), 
								 [](unsigned char x){return std::isspace(x);}),
				  line.end());
		if (line[0] == '#' || line.empty())
			continue;
		auto pos = line.find("=");
		if (pos != std::string::npos) {
			std::string str = line.substr(0, pos);
			if (!map_args_.count(str) && str != BTCLITED_OPTION_CONF) {
				// Don't overwrite existing settings so command line settings override bitcoin.conf
				std::string str_val = line.substr(pos+1);
			}
		}
	}
}

std::string ArgsManager::GetArg(const std::string& arg, const std::string& arg_default) const
{
	LOCK(cs_args_);
	auto it = map_args_.find(arg);
	if (it != map_args_.end())
		return it->second;
	return arg_default;
}

std::vector<std::string> ArgsManager::GetArgs(const std::string& arg) const
{
	LOCK(cs_args_);
	auto it = map_multi_args_.find(arg);
	if (it != map_multi_args_.end())
		return it->second;
	return {};
}

void ArgsManager::SetArg(const std::string& arg, const std::string& arg_val)
{
	LOCK(cs_args_);
	map_args_[arg] = arg_val;
	map_multi_args_[arg] = {arg_val};
}

bool ArgsManager::IsArgSet(const std::string& arg) const
{
	LOCK(cs_args_);
	return map_args_.count(arg);
}

fs::path PathManager::GetDefaultDataDir() const
{
	char *home_path = getenv("HOME");
	if (home_path == NULL)
		return fs::path("/") / ".btclite";
	else
		return fs::path(home_path) / ".btclite";
}

fs::path PathManager::GetDataDir() const
{
	LOCK(cs_path_);
	return path_;
}

void PathManager::UpdateDataDir()
{
	LOCK(cs_path_);
	if (g_args.IsArgSet(BTCLITED_OPTION_DATADIR)) {
		path_ = fs::absolute(g_args.GetArg(BTCLITED_OPTION_DATADIR, ""));
		if (!fs::is_directory(path_)) {
			LogPrint(LogLevel::WARNING, "Specified data directory \"%s\" does not exist. Use default data directory.\n",
					 path_.c_str());
			path_ = GetDefaultDataDir();
		}
	}
}

fs::path PathManager::GetConfigFile(const std::string& conf_path) const
{
	fs::path path(conf_path);
	if (!path.is_absolute())
		path = GetDataDir() / path;
	return path;
}

void SetupEnvironment()
{
	try {
        std::locale(""); // Raises a runtime error if current locale is invalid
    } catch (const std::runtime_error&) {
        setenv("LC_ALL", "C", 1);
    }
}

