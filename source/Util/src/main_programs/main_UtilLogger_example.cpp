
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
    // Get initial values into localopt (this does a copy of the LoggerOptions object).
    Util::LoggerOptions localopt = Util::UtilLogger::getDefaultLoggerOptions();

    // Now set the options needed for this app
    localopt.loglevel = Log::Log::Level::eDebug;
    localopt.logChannelName = logChannelName;
    localopt.logFilelName = logChannelName + "_log.txt";
    localopt.useConsole = Util::MainLogger::disableConsole;

    // This will get us the default set of options from UtilLogger.
    Util::UtilLogger ulogger;
    Util::LoggerOptions logopt;

    {   // braces so we can reuse ostr
        std::stringstream ostr;
        logopt = ulogger.getLoggerOptions();
        ulogger.displayLoggerOptions(ostr, logopt, "-- after construction of UtilLogger from defaults");
        std::cerr << ostr.str() << std::endl;
    }
    {   // braces so we can reuse ostr
        std::stringstream ostr;
        ulogger.setLoggerOptions(localopt);
        logopt = ulogger.getLoggerOptions();
        ulogger.displayLoggerOptions(ostr, logopt, "after setting new options with local values");
        std::cerr << ostr.str() << std::endl;
    }

    // Please note: No additional calls to ulogger.setLoggerOptions() are
    // allowed past this point (an exception will be thrown).
    Util::LoggerSPtr splogger = ulogger.getLoggerPtr();

    {   // braces so we can reuse ostr
        std::stringstream ostr;
        logopt = ulogger.getLoggerOptions();
        ulogger.displayLoggerOptions(ostr, logopt, "after getting shared_ptr<> to Log::Logger");
        splogger->debug() << ostr.str();
    }

    std::string sret = "FAILED";
    try {
        splogger->info() << "Logging first info message";
        splogger->debug() << "Logging first debug message";

        // Set a new value to the log level
        Log::Manager::get(logChannelName.c_str())->setLevel(Log::Log::eInfo);

        // the second debug message should not appear in the log
        splogger->info() << "Logging second info message.";
        splogger->info() << "The following log line should not be a DEBUG level message.";
        splogger->debug() << "Second debug message SHOULD NOT APPEAR IN THE LOG";

        splogger->info() << "Causing intentional exception...";
        ulogger.setLoggerOptions(localopt);
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION: " << e.what() << std::endl;
        splogger->info() << "EXCEPTION: " << e.what();
        sret = "SUCCEEDED";
    }

    std::cerr << sret << std::endl;
    splogger->info() << sret;

    return 0;
}
