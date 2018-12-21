#if defined(HAVE_CONFIG_H)
#include "config/btclite-config.h"
#endif

#include "init.h"
#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"

#include <getopt.h>
#include <stdarg.h>
#include <boost/thread.hpp>

bool output_debug = false;
bool print_to_console = true;
bool print_to_file = false;

std::atomic<uint32_t> g_log_categories(0);


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
void ArgsManger::CheckOptions(int argc, char* const argv[])
{
	for (int i = 1; i < argc; i++) {
		std::string str(argv[i]);
		if (str.length() <= 2 || str.compare(0, 2, "--")) {
			fprintf(stdout, "%s: invalid option '%s'\n", argv[0], str.c_str());
			PrintUsage();
			exit(EXIT_FAILURE);
		}
	}
}

void ArgsManger::ParseParameters(int argc, char* const argv[])
{
	CheckOptions(argc, argv);
	const static struct option options[] {
		{ BTCLITED_OPTION_HELP,  no_argument,       NULL, 'h' },
		{ BTCLITED_OPTION_DEBUG, required_argument, NULL,  0  },
		{ 0,                     0,                 0,     0  }
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
}

std::vector<std::string> ArgsManger::GetArgs(const std::string& arg) const
{
	LOCK(cs_args_);
	auto it = map_multi_args_.find(arg);
	if (it != map_multi_args_.end())
		return it->second;
	return {};
}

bool ArgsManger::IsArgSet(const std::string& arg) const
{
	LOCK(cs_args_);
	return map_args_.count(arg);
}

