// #include <Utility.hpp>
// #include <MainLogger.hpp>

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

int
main()
{
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

