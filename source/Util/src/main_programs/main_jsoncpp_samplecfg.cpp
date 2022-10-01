
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

    	#define ONEWAY
#ifdef ONEWAY

    std::stringstream theoutput;
    ConfigSingletonShrdPtr thesp = ConfigSingleton::create(config_file_name, logger);

    logger.debug() << "\n\nParsed " << config_file_name << " contents: \n"
			       << thesp->instance()->s_configRoot << "\n\nConfig shared_ptr<> use count = " << thesp.use_count() << "\n";

#else // THE OTHER WAY

	logger.debug() << "\n\nParsed " << config_file_name << " contents: \n"
			       << ConfigSingleton::create(config_file_name, logger)->s_configRoot
				   << "\n\nConfig shared_ptr<> use count = " << thesp.use_count() << "\n";
#endif // ONEWAY

    } catch (const std::exception& e) {
    	logger.error() << "Exception while trying to create config singleton: " << e.what();
    	return 1;
    }

    // Do some access:
    // TODO: This may not be safe.  Figure out a way (reference to a const does not ensure no corruption):
    const Json::Value& ro_root = ConfigSingleton::instance()->JsonRoot();

    std::string channel = ro_root["Config"]["Logger"]["channel-name"].asString();
    bool write_to_file = (ro_root["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    int position2 = ro_root["Config"]["position"][1].asInt();
    int frame_count = ro_root["Config"]["Video"]["frame-count"].asInt();

    logger.info() << "\nChannel-name = " << channel << "\n"
    			  << "write-to-file = " << write_to_file << "\n"
				  << "position array index 1 = " << position2 << "\n"
				  << "frame-count = " << frame_count << "\n";

	return 0;
}       

