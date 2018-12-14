#if defined(HAVE_CONFIG_H)
#include "config/btcdemo-config.h"
#endif

#include "init.h"
#include "serialize.h"
#include "sync.h"
#include "util.h"
#include "utiltime.h"

#include <getopt.h>
#include <stdarg.h>
#include <boost/thread.hpp>

CCriticalSection cs_args;
std::map<std::string, std::string> map_args;
static std::map<std::string, std::vector<std::string>> _map_multi_args;
const std::map<std::string, std::vector<std::string>>& map_mutil_args = _map_multi_args;
bool output_debug = false;
bool print_to_console = true;
bool print_to_file = false;

bool LogAcceptCategory(const char* category)
{
    if (category != NULL)
    {
        if (!output_debug)
            return false;

        // Give each thread quick access to -debug settings.
        // This helps prevent issues debugging global destructors,
        // where map_mutil_args might be deleted before another
        // global destructor calls LogPrint()
        static boost::thread_specific_ptr<std::set<std::string> > ptr_category;
        if (ptr_category.get() == NULL)
        {
            if (map_mutil_args.count("debug")) {
                const std::vector<std::string>& categories = map_mutil_args.at("debug");
                ptr_category.reset(new std::set<std::string>(categories.begin(), categories.end()));
                // thread_specific_ptr automatically deletes the set when the thread ends.
            } else
                ptr_category.reset(new std::set<std::string>());
        }
        const std::set<std::string>& set_categories = *ptr_category.get();

        // if not debugging everything and not debugging specific category, LogPrint does nothing.
        if (set_categories.count(std::string("")) == 0 &&
				set_categories.count(std::string("1")) == 0 &&
				set_categories.count(std::string(category)) == 0)
            return false;
    }
    return true;
}

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

void ParseParameters(int argc, char* const argv[])
{
	const static struct option options[] {
		{ "help",  no_argument,       NULL, 'h' },
		{ "debug", required_argument, NULL,  0  },
		{ 0,       0,                 0,     0  }
	};
	int c, option_index;
	
	while ((c = getopt_long(argc, argv, "h?", options, &option_index)) != -1) {
		switch (c) {
			case 0 : 
			{
				std::string str(options[option_index].name);
				std::string str_val(optarg);
				map_args[str] = str_val;
				_map_multi_args[str].push_back(str_val);
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

bool IsArgSet(const std::string& arg)
{
	return map_args.count(arg);
}

