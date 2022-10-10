
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

#include <Utility.hpp>
#include <MainLogger.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>


std::string logChannelName = "main_UtilLogger_example";

int main()
{
    using namespace Util;

    // Get initial values into localopt (this does a copy of the LoggerOptions object).
    LoggerOptions localopt = UtilLogger::getDefaultLoggerOptions();

    // Now set the options needed for this app
    localopt.loglevel = Log::Log::Level::eDebug;
    localopt.logChannelName = logChannelName;
    localopt.logFilelName = logChannelName + "_log.txt";
    localopt.useConsole = MainLogger::disableConsole;
    localopt.useLogFile = MainLogger::enableLogFile;

    // Initialise the UtilLogger object
    UtilLogger::create(localopt);

    // Reference to the logger object
    Log::Logger& ulogger = *(UtilLogger::getLoggerPtr());

    std::string sret = "SUCCEEDED";

    try {
        LoggerOptions logopt;

        std::stringstream ostr;
        logopt = UtilLogger::getLoggerOptions();
        UtilLogger::streamLoggerOptions(ostr, logopt, "after getting shared_ptr<> to Log::Logger");
        ulogger.debug() << ostr.str();

        ulogger.info() << "Logging first info message";
        ulogger.debug() << "Logging first debug message";

        // HOW TO CHANGE LOG LEVEL AFTER LOGGER INITIALIZATION
        // Set a new value to the log level - no more DBUG messages
        Log::Manager::get(logChannelName.c_str())->setLevel(Log::Log::eInfo);

        // the second debug message should not appear in the log
        ulogger.info() << "Logging second info message.";
        ulogger.info() << "The following log line should not be a DEBUG level message.";
        ulogger.debug() << "This second debug message SHOULD NOT APPEAR IN THE LOG";
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        ulogger.info() << "EXCEPTION: " << e.what();
        sret = "FAILED";
    }

    std::cerr << sret << std::endl;
    ulogger.info() << sret;

    return 0;
}
