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


class FullNodeArgs : public Args {
public:
	bool Init(int argc, const char* const argv[]);
	bool InitParameters();

private:
	bool Parse(int argc, const char* const argv[]);
	void PrintUsage();
};

class FullNodeDataFiles : public DataFiles {
public:
	using DataFiles::DataFiles;
	
	bool Init(const std::string& path, const std::string& config_file);
	static fs::path DefaultDataDirPath();
	
};

class FullNodeMain : public Executor {
public:
	FullNodeMain(int argc, const char* const argv[])
		: Executor(argc, argv) , data_files_(FullNodeDataFiles::DefaultDataDirPath(), DEFAULT_CONFIG_FILE) {}
	
	bool Init();
	bool Start();
	bool Run();
	void Interrupt();
	void Stop();
	
private:
	FullNodeArgs args_;
	FullNodeDataFiles data_files_;

	bool InitDataFiles();
	bool LoadConfigFile();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
