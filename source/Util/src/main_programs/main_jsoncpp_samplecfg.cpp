#include <Utility.hpp>
#include <MainLogger.hpp>

// #include <map>
// #include <utility>

#include <iostream>
#include <json/json.h>
#include <fstream>
#include <JsonCppUtil.hpp>

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

    std::string config_file_name = "main_jsoncpp_samplecfg.json";

    // TODO:  Have to continue developing this
	// TODO:  Just for now.
	UtilJsonCpp::jsonsample(config_file_name);

    Json::Reader reader;
    Json::Value cfg_root;
    std::ifstream cfgfile("main_jsoncpp_samplecfg.json");
    if (!cfgfile.is_open())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        std::cerr << "\nERROR: Could not find json file " << config_file_name << ".  Exiting...\n" << std::endl;
        return 1;
    }

    cfgfile >> cfg_root;
    std::cerr << "\n" << cfg_root << std::endl;
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

