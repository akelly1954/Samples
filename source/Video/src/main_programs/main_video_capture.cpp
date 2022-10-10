
// NOTE: TODO: This file is work in progress. Look, but don't run it.

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

#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <JsonCppUtil.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <ConfigSingleton.hpp>
#include <json/json.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>


#ifdef TEST_RAW_CAPTURE_CTL

////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, set frame count from the command line ("-fc 0").
// Un-comment-out this #define if a low-level test of pause/resume/finish capture is needed.
//
// #define TEST_RAW_CAPTURE_CTL

void test_raw_capture_ctl(Log::Logger logger)
{
    logger.debug() << "In test_raw_capture_ctl: thread running";

    for (int i = 1; i <= 3; i++)
    {
        ::sleep(3);
        logger.debug() << "test_raw_capture_ctl: PAUSING CAPTURE: " << i;
        ::set_v4l2capture_pause(true);

        ::sleep(3);
        logger.debug() << "test_raw_capture_ctl: RESUMING CAPTURE: " << i;
        ::set_v4l2capture_pause(false);
    }

    ::sleep(3);
    logger.debug() << "test_raw_capture_ctl: FINISH CAPTURE REQUEST...";
    ::set_v4l2capture_finished();
}

#endif // TEST_RAW_CAPTURE_CTL

// CONFIGURATION
//
// Configuration of this program is affected by the initial (compiled) defaults of certain
// specific objects that can at run time be overwritten with values obtained from json configuration
// file, as well as command line parameters.
//
// The order of precedence is simple:  If nothing else, the compiled static default values of
// objects will take effect.  Next, values of json config parameters will overwite whatever
// is in effect when they are read in.  Next, values of command line parameters overwrite whatever
// is in effect after the json values are read in.
//
// Once the json config is read in at run time, any value which comes from the json file,
// will overwrite the default value that the program was compiled with for the specific object
// being handled (lets say, the logger debug level - "DEBG", "INFO", etc).
//
// After the json config values have been written into the specifc objects they are meant
// for, command line parameters are parsed and updated into these same objects for those
// objects that are configurable from the command line.
//
// As an example:
//
//         The logger's log-level starts out in video_capture_globals.cpp defined as
//         Video::vcGlobals::loglevel, and initialized to some Log::Log::Level value, such
//         as Log::Log::Level::eDebug.
//
//         Next comes the json config file.  If the following section exists, and contains
//         a valid value, it will overwrite the object Video::vcGlobals::loglevel:
//
//         {
//             "Config": {
//                  . . . . . .
//                     "Logger": {
//                        . . . . . .
//                         "log-level":        "INFO"
//                        . . . . . .
//                     },
//                  . . . . . .
//             }
//         }
//
//         As a result of the above section, the variable Video::vcGlobals::loglevel will be
//         assigned the value Log::Log::Level::eInfo replacing compiled static value.
//
//         At the end, if the command line option "-lg NOTE" is specified, the same variable -
//         Video::vcGlobals::loglevel will finally be set to Log::Log::Level::eNotice, erasing
//         the effects of all previous values.
//

// static definitions

Util::LoggerOptions setLocalLoggerOptions()
{
    Util::LoggerOptions localopt = {
            // loglevel (in LoggerOptions)
            Video::vcGlobals::loglevel,

            // log_level (in LoggerOptions)
            Video::vcGlobals::log_level,

            // logChannelName (in LoggerOptions)
            Video::vcGlobals::logChannelName,

            // logFileName (in LoggerOptions)
            Video::vcGlobals::logFilelName,

            // useConsole  (in LoggerOptions)
            Util::MainLogger::disableConsole,

            // useLogFile (in LoggerOptions)
            Util::MainLogger::enableLogFile
    };
    return localopt;
}

int main(int argc, const char *argv[])
{
    using namespace Video;

    // Setting up the config singleton before the logger is set
    // up - we will accumulate logger lines until it is.
    std::vector<std::string> delayedLinesForLogger;

    /////////////////
    // Parse the command line part 1 (print out the --help info asap)
    /////////////////

    std::string argv0 = argv[0];
    std::string argv1 = (argv[1] == nullptr? "-h" : argv[1]);

    // If no parameters were supplied, or help was requested:
    if (argc > 1 && (argv1 == "--help" || argv1 == "-h" || argv1 == "help"))
    {
        Video::CommandLine::Usage(std::cerr, argv0);
        std::cerr << std::endl;
        if (argc > 2)
        {
            std::cerr << "WARNING: using the --help flag negates consideration of all other flags and parameters.  Exiting...\n" << std::endl;
        }

        return EXIT_SUCCESS;
    }

    /////////////////
    // Set up the application configuration:
    //
    // Before we do the actual parsing of the command line, the initial json/config
    // has to be done so that the command line can overwrite its values in the following step.
    /////////////////

    std::stringstream loggerStream;

    // declaring this outside the scope of try/catch so it can be used later
    Config::ConfigSingletonShrdPtr thesp;

    try {
        /////////////////
        // Set up config
        /////////////////

        thesp = Config::ConfigSingleton::create(Video::vcGlobals::config_file_name, loggerStream);

        // At this point the json root node has been set up - after parsing, checking syntax, etc.
        // If ANY errors are encountered along the way, they will be catch()ed below and the
        // program aborted.
        //
        // As of this point, the root node can be accessed by reference with
        //
        //              Json::Value& ConfigSingleton::instance()->JsonRoot();
        //

    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception while trying to create config singleton: \n    " << e.what() << std::endl;
        std::cerr << "Previously logged info: " << loggerStream.str() << std::endl;
        return EXIT_FAILURE;
    }

    // We will log this as soon as the logger is configured and operational.
    std::string parse_output = std::string("\n\nParsed JSON nodes:") + loggerStream.str();

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(parse_output);

    std::stringstream configstrm;
    configstrm << "\n\nParsed JSON file " << Video::vcGlobals::config_file_name << " successfully.  Contents: \n\n"
                      << Config::ConfigSingleton::instance()->JsonRoot() << "\n";

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(configstrm.str());

    try {

        std::stringstream strm;
        if (! Video::updateInternalConfigsWithJsonValues(strm, Config::ConfigSingleton::instance()->JsonRoot()))
        {
            std::cerr << strm.str() << std::endl;
            std::cerr << "\nError while accessing json values read from " << Video::vcGlobals::config_file_name << "." << std::endl;
            return EXIT_FAILURE;
        }

        std::string constring = strm.str();
        std::cerr << constring << std::endl;

        // Everything in the vector will be written to the log file
        // as soon as the logger is initialized.
        delayedLinesForLogger.push_back(constring);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error:  Exception caught while accessing json values read from "
                  << Video::vcGlobals::config_file_name << ": "
                  << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    /////////////////
    // Parse the command line part 2
    /////////////////

    std::stringstream lstrm;
    if (! Video::CommandLine::parse(lstrm, argc, argv))
    {
        Video::CommandLine::Usage(std::cerr, argv0);
        std::cerr << "Command line parsing:\n" << lstrm.str() << std::endl;
        return EXIT_FAILURE;
    }

    std::string lstr = std::string("Command line parsing:\n") + lstrm.str();
    std::cerr << lstr << std::endl;
    delayedLinesForLogger.push_back(lstr);

    /////////////////
    // Set up the logger
    /////////////////

    // This picks the values from Video::vcGlobals (which was modified
    // by the json file and then the command line.
    Util::LoggerOptions localopt = setLocalLoggerOptions();

    // set the values in localopt as the values in ulogger.
    Util::UtilLogger::setLoggerOptions(localopt);

    //////////////////////////////////////////////////////////////
    // Initialise the UtilLogger object
    //////////////////////////////////////////////////////////////
    Util::UtilLogger::create(localopt);

    //////////////////////////////////////////////////////////////
    // Reference to THE logger object
    //////////////////////////////////////////////////////////////
    Log::Logger& ulogger = *(Util::UtilLogger::getLoggerPtr());

    {
        Util::LoggerOptions logopt;

        std::stringstream ostr;
        logopt = Util::UtilLogger::getLoggerOptions();
        Util::UtilLogger::streamLoggerOptions(ostr, logopt, "after getting shared_ptr<> to Log::Logger");
        ulogger.debug() << ostr.str();
    }

    // The logger is now set up.
    ulogger.info() << "\n\nLogger setup is complete.\n";
    ulogger.info() << "    **********  Deferred output from app initialization (only displayed in DBUG mode.  **********";
    ulogger.info() << "    **********          Need to run with \"-lg DBUG\" on the command line).            **********";
    ulogger.info() << "";


    // Empty out the delayed-lines' vector...
    for(auto line : delayedLinesForLogger)
    {
        ulogger.debug() << line;
    }

    return 0;
}

