#if defined(HAVE_CONFIG_H)
#include "config/btcdemo-config.h"
#endif

#include "sync.h"
#include "util.h"
#include "serialize.h"

#include <stdarg.h>

CCriticalSection cs_args;
std::map<std::string, std::string> map_args;
static std::map<std::string, std::vector<std::string>> _map_multi_args;
const std::map<std::string, std::vector<std::string>>& map_mutil_args = _map_multi_args;

void ParseParameters(int argc, const char* const argv[])
{
	LOCK(cs_args);
	map_args.clear();
	_map_multi_args.clear();
	
	for (int i = 1; i < argc; i++) {
		if (*argv[i] != '-')
			break;
		std::string str(argv[i]);
		std::string str_val;
		std::string::size_type n = str.find("=");
		if (n != std::string::npos) {
			str_val = str.substr(n+1);
			str = str.substr(0, n);
		}
		
		if (str.length() > 1 && str[1] == '-')
			str = str.substr(1);
		map_args[str] = str_val;
		_map_multi_args[str].push_back(str_val);
	}
}

bool IsArgSet(const std::string& arg)
{
	return map_args.count(arg);
}

std::string HelpMessageGroup(const std::string &message) {
    return std::string(message) + std::string("\n\n");
}

std::string HelpMessageOpt(const std::string &option, const std::string &message) {
return std::string("  ") + std::string(option) +
           std::string("\n        ") + std::string(message) + std::string("\n\n");
}
