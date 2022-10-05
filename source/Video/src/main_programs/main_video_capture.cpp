
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>

// This is the basis for the default logger, log file name, etc.


// statics

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

int main(int argc, const char *argv[])
{
    using namespace Video;

    /////////////////
    // Parse the command line
    /////////////////

    std::string argv0 = argv[0];

    // If no parameters were supplied, or help was requested:
    if (argc > 1 && (std::string(const_cast<const char*>(argv[1])) == "--help"
                    || std::string(const_cast<const char*>(argv[1])) == "-h"
                    || std::string(const_cast<const char*>(argv[1])) == "help")
            )
    {
        Video::CommandLine::Usage(std::cerr, argv0); std::cerr << std::endl;
        if (argc > 2)
        {
            std::cerr << "WARNING: using the --help flag negates consideration of all other flags and parameters.  Exiting...\n" << std::endl;
        }

        return EXIT_SUCCESS;
    }

    if (! Video::CommandLine::parse(std::cerr, argc, argv))
    {
        Video::CommandLine::Usage(std::cerr, argv0);  std::cerr << std::endl;
        return EXIT_FAILURE;
    }

    /////////////////
    // Set up the logger
    /////////////////

    // This become home of *the* logger.
    Util::UtilLogger ulogger;

    // Get initial values into localopt (this does a copy of the LoggerOptions object).
    Util::LoggerOptions localopt = Util::UtilLogger::getDefaultLoggerOptions();

    // the default values set by class Util::UtilLogger are now overriden by
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

    return 0;
}

