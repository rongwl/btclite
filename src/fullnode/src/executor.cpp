#include "executor.h"


bool FullNodeMain::Init()
{
    if (!args_.Init(argc(), argv())) 
        return false;

    if (!InitDataFiles())
        return false;

    if (!LoadConfigFile())
        return false;

    if (!args_.InitParameters())
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
    BTCLOG(LOG_LEVEL_INFO) << __func__ << ": progress...";
    
    BTCLOG(LOG_LEVEL_INFO) << __func__ << ": done";
}

bool FullNodeMain::InitDataFiles()
{
    std::string path = FullNodeDataFiles::DefaultDataDirPath().c_str();
    if (args_.IsArgSet(GLOBAL_OPTION_DATADIR))
        path = args_.GetArg(GLOBAL_OPTION_DATADIR, DEFAULT_DATA_DIR);
    
    std::string config_file = args_.GetArg(GLOBAL_OPTION_CONF, DEFAULT_CONFIG_FILE);
    if (!data_files_.Init(path, config_file))
        return false;
    
    if (!data_files_.LockDataDir())
        return false;
    
    return true;
}

bool FullNodeMain::LoadConfigFile()
{
    if (!fs::is_directory(data_files_.data_dir())) {
        BTCLOG(LOG_LEVEL_ERROR) << "Error: Specified data directory \"" << data_files_.data_dir().c_str() << "\" does not exist.";
        return false;
    }
    
    return args_.ParseFromFile(data_files_.config_file().c_str());
}

bool FullNodeMain::InitNetwork()
{
    NetworkEnv env;
    
    if (args_.IsArgSet(GLOBAL_OPTION_TESTNET))
        env = NetworkEnv::testnet;
    else if (args_.IsArgSet(GLOBAL_OPTION_REGTEST))
        env = NetworkEnv::regtest;
    else
        env = NetworkEnv::mainnet;
        
    return network_.Init(env);
}


