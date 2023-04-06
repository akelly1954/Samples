
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

#include <logger_tools.hpp>
#include <video_capture_globals.hpp>

void Video::setup_video_capture_logger(const std::string& cmdline, std::vector<std::string>& delayedLinesForLogger)
{
    using namespace Video;

    // Remove an existing log file before instantiating the logger:
    // This can only be done after command line parsing is finished.
    ::unlink(vcGlobals::logFilelName.c_str());

    // This picks the values from Video::vcGlobals (which was modified
    // by the json file and then the command line.
    Util::LoggerOptions localopt = Util::UtilLogger::setLocalLoggerOptions(
                                                vcGlobals::logChannelName,
                                                vcGlobals::loglevel,
                                                Util::MainLogger::disableConsole,
                                                Util::MainLogger::enableLogFile
                                        );

    //////////////////////////////////////////////////////////////
    // Initialize the UtilLogger object
    //////////////////////////////////////////////////////////////
    Util::UtilLogger::create(localopt);

    //////////////////////////////////////////////////////////////
    // Reference to THE logger object
    //////////////////////////////////////////////////////////////
    std::shared_ptr<Log::Logger> uloggerp = Util::UtilLogger::getLoggerPtr();

    //////////////////////////////////////////////////////////////
    // Start Logging
    //////////////////////////////////////////////////////////////

    uloggerp->info() << "START OF NEW VIDEO CAPTURE RUN";
    uloggerp->info() << "Command line: " << cmdline << "\n";

    if (vcGlobals::log_initialization_info)     // -loginit flag
    {
        Util::LoggerOptions logopt;
        std::stringstream ostr;
        logopt = Util::UtilLogger::getLoggerOptions();

        Util::UtilLogger::streamLoggerOptions(ostr, logopt, "after defining the instance of Log::Logger");
        uloggerp->debug() << ostr.str();
    }

    // The logger is now set up.
    uloggerp->info() << "\n\nLogger setup is complete.\n";
    uloggerp->info() << "";

    if (vcGlobals::log_initialization_info)     // -loginit flag
    {
        uloggerp->info() << "\n\n    ******  Deferred output from app initialization:  ******\n";

        // Empty out the delayed-lines' vector...
        for(auto line : delayedLinesForLogger)
        {
            uloggerp->info() << "\n\nDELAYED: " << line;
        }
    }
    else
    {
        // -loginit flag was not specified: Capture the last few lines into the log file
        uloggerp->info() << "Output to the logger during initialization is not shown here. For the full   ******";
        uloggerp->info() << "set of deferred log lines, use the -loginit flag on the command line.        ******";
        uloggerp->info() << "";
    }
}
