#ifndef BTCLITE_FULLNODE_EXECUTOR_H
#define BTCLITE_FULLNODE_EXECUTOR_H

#include "fullnode/include/config.h"


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
	Fullnode::Configuration configuration_;

	bool InitDataFiles();
	bool LoadConfigFile();
};


#endif // BTCLITE_FULLNODE_EXECUTOR_H
