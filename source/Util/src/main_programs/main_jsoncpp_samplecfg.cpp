
/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 Andrew Kelly
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
/////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <json/json.h>
#include <fstream>
#include <sstream>
#include <MainLogger.hpp>
#include <ConfigSingleton.hpp>

// Logger statics
std::string logChannelName = "jsoncpp_samplecfg";
std::string logFilelName = logChannelName + "_log.txt";
Log::Log::Level loglevel = Log::Log::Level::eDebug;
std::string default_log_level = "debug";
std::string log_level = default_log_level;

// Config statics
std::string config_file_name = "main_jsoncpp_samplecfg.json";

int main(int argc, char *argv[])
{
	using namespace Config;

    /////////////////
    // Set up the logger
    /////////////////

    Log::Config::Vector configList;
    Util::MainLogger::initializeLogManager(configList, loglevel, logFilelName, Util::MainLogger::disableConsole, Util::MainLogger::enableLogFile);
    Util::MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName.c_str());

    std::cerr << "Log level is: " << log_level << std::endl;

    try {
        /////////////////
        // Set up the config
        /////////////////

		std::stringstream theoutput;
		ConfigSingletonShrdPtr thesp = ConfigSingleton::create(config_file_name, logger);

		logger.debug() << "\n\nConfig instance shared_ptr<> use count = " << thesp.use_count() << "\n"
					   << "\nParsed " << config_file_name << " contents: \n"
					   << thesp->instance()->s_configRoot << "\n\n";

    } catch (const std::exception& e) {
    	logger.error() << "Exception while trying to create config singleton: " << e.what();
    	return 1;
    }

    // Do some access:
    Json::Value& ref_root = ConfigSingleton::instance()->JsonRoot();

    std::string channel = ref_root["Config"]["Logger"]["channel-name"].asString();
    bool write_to_file = (ref_root["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    int position2 = ref_root["Config"]["position"][1].asInt();
    int frame_count = ref_root["Config"]["Video"]["frame-count"].asInt();

    logger.info() << "\nChannel-name = " << channel << "\n"
    			  << "write-to-file = " << write_to_file << "\n"
				  << "position array index 1 = " << position2 << "\n"
				  << "frame-count = " << frame_count << "\n";

    logger.info() << "\n\nAfter modifications:\n";
    ref_root["Config"]["Logger"]["channel-name"] = std::string("newChannelName");
    ref_root["Config"]["App-options"]["write-to-file"] = 0;
    ref_root["Config"]["position"][1] = 1246;
    ref_root["Config"]["Video"]["frame-count"] = 305;

    channel = ref_root["Config"]["Logger"]["channel-name"].asString();
    write_to_file = (ref_root["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    position2 = ref_root["Config"]["position"][1].asInt();
    frame_count = ref_root["Config"]["Video"]["frame-count"].asInt();

    logger.info() << "\nChannel-name = " << channel << "\n"
    			  << "write-to-file = " << write_to_file << "\n"
				  << "position array index 1 = " << position2 << "\n"
				  << "frame-count = " << frame_count << "\n";

    // Write out a new file with the modified root.

    std::string newfilename = std::string("new_") + ConfigSingleton::JsonFileName();
	std::ofstream newcfgfile(newfilename, std::ofstream::trunc | std::ifstream::out);
	if (!newcfgfile.is_open())
	{
		// JsonCpp does not check this, but will fail with a syntax error on the first read
		logger.error() << "\nERROR: Could not open the new json file " << newfilename << ".  Exiting...\n";
    	std::cerr << "\nERROR: Could not open the new json file " << newfilename << ".  Exiting...\n" << std::endl;
		newcfgfile.close();
		return 1;
	}

	newcfgfile << ref_root;
	newcfgfile.close();
	return 0;
























	return 0;
}       

