#pragma once

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
#include <LoggerCpp/LoggerCpp.h>
#include <string>
#include <memory>
#include <mutex>

namespace Util {

    class MainLogger
    {
    public:
        enum ConsoleOutput      // flag for initializeLogManager() method
        {
            disableConsole = 0,
            enableConsole
        };

        enum UseLogFile          // flag for initializeLogManager() method
        {
            disableLogFile = 0,
            enableLogFile
        };
    public:
        static void initialize( const std::string& channel_name,
                                Log::Log::Level level,
                                MainLogger::ConsoleOutput useConsole,
                                MainLogger::UseLogFile useLogFile
                               );

        static void initializeLogManager(  Log::Config::Vector& configList,
                                            Log::Log::Level loglevel,
                                            const std::string& logfilename,
                                            ConsoleOutput useConsole,
                                            UseLogFile useLogFile
                                         );

        static void configureLogManager( Log::Config::Vector& configList, std::string channelName );

        static std::mutex s_logger_mutex;

        static std::string logChannelName;
        static std::string logFilelName;
        static Log::Log::Level loglevel;
        static std::string default_log_level;
        static std::string log_level;
    };

} // end of namespace Util
