
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
std::string     logChannelName =    "jsoncpp_samplecfg";
Log::Log::Level loglevel =          Log::Log::Level::eDebug;
std::string     log_level =         Log::Log::toString(loglevel);
std::string     jsonFileName =      std::string("main_") + logChannelName + std::string(".json");

int main(int argc, char *argv[])
{
    using namespace Config;

    std::stringstream loggerStream;

    // declaring this outside the scope of try/catch so it can be used later
    ConfigSingletonShrdPtr thesp;

    try {
        /////////////////
        // Set up the config
        /////////////////

        thesp = ConfigSingleton::create(jsonFileName, loggerStream);

    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception while trying to create config singleton: \n    " << e.what() << std::endl;
        std::cerr << "Previously logged info: " << loggerStream.str() << std::endl;
        return EXIT_FAILURE;
    }

    /////////////////
    // Set up the logger
    /////////////////
    Util::LoggerOptions localopt = Util::UtilLogger::setLocalLoggerOptions(
                                                        logChannelName,
                                                        loglevel,
                                                        Util::MainLogger::disableConsole,
                                                        Util::MainLogger::enableLogFile
                                                    );
    Util::UtilLogger::create(localopt);
    Util::LoggerSPtr loggerp = Util::UtilLogger::getLoggerPtr();

    std::cerr << "Log level is: " << log_level << std::endl;

    loggerp->debug() << "Config instance shared_ptr<> use count = " << thesp.use_count()
                     << "\nParsed " << jsonFileName << " contents: \n"
                     << thesp->instance()->JsonRoot();

    loggerp->notice() << "Items logged before logger initialization:\n";
    loggerp->notice() << loggerStream.str();

    /////////////////
    // Do some access:
    /////////////////

    Json::Value& ref_root_copy = ConfigSingleton::GetJsonRootCopyRef();

    std::string channel = ref_root_copy["Config"]["Logger"]["channel-name"].asString();
    bool write_to_file = (ref_root_copy["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    int position2 = ref_root_copy["Config"]["position"][1].asInt();
    int frame_count = ref_root_copy["Config"]["Video"]["frame-count"].asInt();

    loggerp->info()  << "\nChannel-name = " << channel << "\n"
                  << "write-to-file = " << write_to_file << "\n"
                  << "position array index 1 = " << position2 << "\n"
                  << "frame-count = " << frame_count << "\n";

    loggerp->info() << "After modifications:\n";
    ref_root_copy["Config"]["Logger"]["channel-name"] = std::string("newChannelName");
    ref_root_copy["Config"]["App-options"]["write-to-file"] = 0;
    ref_root_copy["Config"]["position"][1] = 1246;
    ref_root_copy["Config"]["Video"]["frame-count"] = 305;

    channel = ref_root_copy["Config"]["Logger"]["channel-name"].asString();
    write_to_file = (ref_root_copy["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    position2 = ref_root_copy["Config"]["position"][1].asInt();
    frame_count = ref_root_copy["Config"]["Video"]["frame-count"].asInt();

    loggerp->info()  << "\nChannel-name = " << channel << "\n"
                  << "write-to-file = " << write_to_file << "\n"
                  << "position array index 1 = " << position2 << "\n"
                  << "frame-count = " << frame_count << "\n";

    // Write out a new file with the modified root.

    std::string newfilename = std::string("new_") + ConfigSingleton::JsonFileName();
    std::ofstream newcfgfile(newfilename, std::ofstream::trunc | std::ofstream::out);
    if (!newcfgfile.is_open())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        loggerp->error() << "ERROR: Could not open/create the new json file " << newfilename << ".  Exiting...\n";
        std::cerr << "\nERROR: Could not open/create the new json file " << newfilename << ".  Exiting...\n" << std::endl;
        newcfgfile.close();
        return 1;
    }

    newcfgfile << ref_root_copy;
    newcfgfile.close();

    //////////////////////////////////////////
    // Checking out some object methods
    //////////////////////////////////////////

    loggerp->info() << "      ****** CHECKING OUT SOME OBJECT METHODS ******\n";

    std::stringstream root_strm;
    root_strm << ref_root_copy;
    std::string newroot = root_strm.str();

    loggerp->info() << "Streamed existing ref_root_copy to strstream:\n" << newroot ;

    Json::Value new_root = ref_root_copy;

    std::stringstream newroot_strm;
    newroot_strm << new_root;
    newroot = newroot_strm.str();

    loggerp->info() << "Streamed the new root to stringstream:" << "\n" << newroot ;

    channel = new_root["Config"]["Logger"]["channel-name"].asString();
    write_to_file = (new_root["Config"]["App-options"]["write-to-file"].asBool()) == 0? false: true;
    position2 = new_root["Config"]["position"][1].asInt();
    frame_count = new_root["Config"]["Video"]["frame-count"].asInt();

    loggerp->info() << "Values from copied editable root reference:\n"
                  << "Channel-name = " << channel << "\n"
                  << "write-to-file = " << write_to_file << "\n"
                  << "position array index 1 = " << position2 << "\n"
                  << "frame-count = " << frame_count << "\n";

    //////////////////////////////////////////
    // Checking out UpdateJsonConfigFile()
    //////////////////////////////////////////

    loggerp->info() << "      ****** CHECKING OUT UpdateJsonConfigFile() ******";

    // At this point new_root has a version of the modified root json node
    // Copy it to our reference to the singleton's s_editRoot member.
    ref_root_copy = new_root;

    Json::Value& edit_root_ref = ConfigSingleton::GetJsonRootCopyRef();
    loggerp->info() << "Contents of s_editRoot:\n" << edit_root_ref ;

    std::stringstream log2Stream;
    if (! ConfigSingleton::instance()->UpdateJsonConfigFile(log2Stream))
    {
        std::string previousLog = log2Stream.str();
        loggerp->error() << "ERROR: UpdateJsonConfigFile() failed.  Exiting... \n";
        loggerp->error() << "Contents of UpdateJsonConfigFile() logging:\n" << previousLog << "\n";
        std::cerr << "\nERROR: UpdateJsonConfigFile() failed.  Exiting... " << previousLog << std::endl;
        return 1;
    }

    loggerp->debug() << "Contents of UpdateJsonConfigFile() logging:\n" << log2Stream.str() << "\n";
    return 0;
}


