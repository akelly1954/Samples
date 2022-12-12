
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

Util::LoggerOptions Util::setLocalLoggerOptions()
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

void Video::setup_video_capture_logger( std::string& ParseOutputString,
                                        std::string& ConfigOutputString,
                                        std::vector<std::string>& delayedLinesForLogger)
{
    using namespace Video;

    // Remove an existing log file before instantiating the logger:
    // This can only be done after command line parsing is finished.
    ::unlink(Video::vcGlobals::logFilelName.c_str());

    // This picks the values from Video::vcGlobals (which was modified
    // by the json file and then the command line.
    Util::LoggerOptions localopt = Util::setLocalLoggerOptions();

    // set the values in localopt as the values in ulogger.
    Util::UtilLogger::setLoggerOptions(localopt);

    //////////////////////////////////////////////////////////////
    // Initialize the UtilLogger object
    //////////////////////////////////////////////////////////////
    Util::UtilLogger::create(localopt);

    //////////////////////////////////////////////////////////////
    // Reference to THE logger object
    //////////////////////////////////////////////////////////////
    Log::Logger ulogger = *(Util::UtilLogger::getLoggerPtr());

    //////////////////////////////////////////////////////////////
    // Start Logging
    //////////////////////////////////////////////////////////////

    ulogger.info() << "START OF NEW VIDEO CAPTURE RUN";

    {
        Util::LoggerOptions logopt;
        std::stringstream ostr;
        logopt = Util::UtilLogger::getLoggerOptions();

        Util::UtilLogger::streamLoggerOptions(ostr, logopt, "after defining the instance of Log::Logger");
        ulogger.debug() << ostr.str();
    }

    // The logger is now set up.
    ulogger.info() << "\n\nLogger setup is complete.\n";
    ulogger.info() << "";

    if (vcGlobals::log_initialization_info)     // -loginit flag
    {
        ulogger.info() << "\n\n    ******  Deferred output from app initialization:  ******\n";

        // Empty out the delayed-lines' vector...
        for(auto line : delayedLinesForLogger)
        {
            ulogger.info() << "DELAYED: " << line;
        }
    }
    else
    {
        // -loginit flag was not specified: Capture the last few lines into the log file
        ulogger.info() << "The last few lines of deferred output from app initialization are shown here.   ******";
        ulogger.info() << "For the full set of deferred lines, use the -loginit flag on the command line.  ******";

        ulogger.info() << "DELAYED: .  .  .  . . . .\n" << ConfigOutputString << "\n";
        ulogger.info() << "DELAYED: .  .  .  . . . .\n\n" << ParseOutputString;
    }
}
