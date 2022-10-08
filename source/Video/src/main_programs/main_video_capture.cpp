
// NOTE: TODO: This file has work in progress. Do not use.

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>


// static definitions

static Util::LoggerOptions defaultLogOptions = {
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


////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, set frame count from the command line ("-fc 0").
// Un-comment-out this #define if a low-level test of pause/resume/finish capture is needed.
//
// #define TEST_RAW_CAPTURE_CTL

#ifdef TEST_RAW_CAPTURE_CTL

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


int main(int argc, const char *argv[])
{
    using namespace Video;

    std::vector<std::string> delayedLinesForLogger;

    /////////////////
    // Parse the command line part 1 (print out the --help info asap)
    /////////////////

    std::string argv0 = argv[0];
    std::string argv1 = argv[1];

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

    // This is the actual root node in the internal json tree.
    Json::Value cfg_root;

    std::ifstream ifs(Video::vcGlobals::config_file_name, std::ios::in);
    if (!ifs.is_open() || !ifs.good()) {
        const char *errorStr = strerror(errno);
        std::cerr << "Failed to open Json file " << Video::vcGlobals::config_file_name.c_str() << " " << errorStr << std::endl;
        return EXIT_FAILURE;
    }

    std::ostringstream strm;
    if (UtilJsonCpp::checkjsonsyntax(strm, ifs, cfg_root) == EXIT_FAILURE)
    {
        std::string errstr = strm.str();
        std::cerr << "\nERROR in file " << Video::vcGlobals::config_file_name << ":\n\n" << errstr << std::endl;
        if (ifs.is_open()) ifs.close();
        return EXIT_FAILURE;
    }
    if (ifs.is_open()) ifs.close();

    // We will log this as soon as the logger is configured and operational.
    std::string parse_output = std::string("\n\nParsed JSON nodes:") + strm.str();

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(parse_output);

    std::ifstream cfgfile(Video::vcGlobals::config_file_name);
    if (!cfgfile.is_open())
    {
        // This message is for debugging help. JsonCpp does not check if the file stream is
        // valid, but will fail with a syntax error on the first read, which is not helpful.
        std::cerr << "\nERROR: Could not find json file " << Video::vcGlobals::config_file_name << ".  Exiting...\n" << std::endl;
        return EXIT_FAILURE;
    }
    ///////////////////////   TODO:  cfgfile >> cfg_root;    // json operator>>()

    std::stringstream configstrm;
    configstrm << "\n\nParsed JSON file " << Video::vcGlobals::config_file_name << " successfully.  Contents: \n\n"
                      << cfg_root << "\n";

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(configstrm.str());

    cfgfile.close();

    ///////////////////////////////////////////////////////////////////////////////////
    // As of this point, cfg_root contains the root for the whole json tree.
    ///////////////////////////////////////////////////////////////////////////////////

    try {

        std::stringstream strm;
        if (! Video::updateInternalConfigsWithJsonValues(strm, cfg_root))
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

    if (! Video::CommandLine::parse(std::cerr, argc, argv))
    {
        Video::CommandLine::Usage(std::cerr, argv0);
        std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    /////////////////
    // Set up the logger
    /////////////////

    // This become home of *the* logger.
    Util::UtilLogger ulogger;

    // Get initial values into localopt (this does a copy of the LoggerOptions object).
    Util::LoggerOptions localopt = Util::UtilLogger::getDefaultLoggerOptions();

    // the default values set by class Util::UtilLogger are now overwritten by
    // the defaultLogOptions struct declared above.
    localopt = defaultLogOptions;

    // At this point, the individual struct members can be assigned directly (in localopt)
    // as well, for example:
    //          localopt.logFilelName = "somestring.txt";

    // set the values in localopt as the values in ulogger.
    ulogger.setLoggerOptions(localopt);

    // No additional calls to ulogger.setLoggerOptions() are
    // allowed past this point (an exception will be thrown).
    // This statement is the same as:
    //      std::shared_ptr<Log::Logger> splogger = ulogger.getLoggerPtr();
    Util::LoggerSPtr splogger = ulogger.getLoggerPtr();

    // The logger is now set up.
    splogger->info() << "Logger setup is complete.";

    // Empty out the delayed-lines' vector...
    for(auto line : delayedLinesForLogger)
    {
        splogger->info() << line;
    }

    return 0;
}

