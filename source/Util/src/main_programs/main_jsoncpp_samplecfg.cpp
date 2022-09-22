#include <Utility.hpp>
#include <MainLogger.hpp>

// #include <map>
// #include <utility>

#include <iostream>
#include <json/json.h>
#include <fstream>

//
// main_jsoncpp_samplecfg.cpp
// 
// Taken from https://github.com/sksodhi/CodeNuggets/tree/master/json/config_read
// 
// See MIT License for copyright for this code:
//           https://github.com/sksodhi/CodeNuggets/blob/master/LICENSE
//

void 
displayCfg(const Json::Value &cfg_root, Log::Logger logger);

int
main()
{
	using namespace Util;

	std::string channelName = "main_jsoncpp_samplecfg";

    Log::Config::Vector configList;
    MainLogger::initialize( configList,
                            channelName,
                            Log::Log::Level::eDebug,
                            MainLogger::enableConsole,
                            MainLogger::disableLogFile
                          );

    Log::Logger logger(channelName.c_str());

    Json::Reader reader;
    Json::Value cfg_root;
    std::ifstream cfgfile("main_jsoncpp_samplecfg.json");
    cfgfile >> cfg_root;

    logger.notice() << "--------- cfg_root : start ---------";
    logger.notice() << cfg_root;
    logger.notice() << "--------- cfg_root : end ---------";

    displayCfg(cfg_root, logger);
}       

void 
displayCfg(const Json::Value &cfg_root, Log::Logger logger)
{
    std::string serverIP = cfg_root["Config"]["server-ip"].asString();
    std::string serverPort = cfg_root["Config"]["server-port"].asString();
    unsigned int bufferLen = cfg_root["Config"]["buffer-length"].asUInt();

    logger.notice() << "--------- Configuration ---------";
    logger.notice() << "server-ip     :" << serverIP;
    logger.notice() << "server-port   :" << serverPort;
    logger.notice() << "buffer-length :" << bufferLen;
}

