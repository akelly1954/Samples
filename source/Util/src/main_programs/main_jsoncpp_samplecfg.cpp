//
// main_jsoncpp_samplecfg.cpp
// 
// Taken from https://github.com/sksodhi/CodeNuggets/tree/master/json/config_read
// 
// See MIT License for copyright for this code:
//           https://github.com/sksodhi/CodeNuggets/blob/master/LICENSE
//
#include <iostream>
#include <json/json.h>
#include <fstream>

void 
displayCfg(const Json::Value &cfg_root);

int
main()
{
    Json::Reader reader;
    Json::Value cfg_root;
    std::ifstream cfgfile("main_jsoncpp_samplecfg.json");
    cfgfile >> cfg_root;

    std::cout << "______ cfg_root : start ______" << std::endl;
    std::cout << cfg_root << std::endl;
    std::cout << "______ cfg_root : end ________" << std::endl;

    displayCfg(cfg_root);
}       

void 
displayCfg(const Json::Value &cfg_root)
{
    std::string serverIP = cfg_root["Config"]["server-ip"].asString();
    std::string serverPort = cfg_root["Config"]["server-port"].asString();
    unsigned int bufferLen = cfg_root["Config"]["buffer-length"].asUInt();

    std::cout << "______ Configuration ______" << std::endl;
    std::cout << "server-ip     :" << serverIP << std::endl;
    std::cout << "server-port   :" << serverPort << std::endl;
    std::cout << "buffer-length :" << bufferLen<< std::endl;
}

