#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H

#include "environment.h"
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


class FullNodeArgs : public ArgsManager {
public:
	bool Parse(int argc, char* const argv[]);
	void PrintUsage();

};

class FullNodeDataFiles : public DataFilesManager {
public:
	FullNodeDataFiles(const std::string& path);
};

class FullNodeMain : public Executor {
public:
	FullNodeMain(FullNodeArgs& args, FullNodeDataFiles& data_files) : Executor(args, data_files) {}
	
	bool Init();
	bool Start();
	bool Run();
	void Interrupt();
	void Stop();
	
private:
	bool CheckDataPath();
	bool InitConfigFile();
	bool InitParameters();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
