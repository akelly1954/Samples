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

namespace Util
{
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
        static void initialize( Log::Config::Vector& configList,
                                const std::string& channel_name,
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

    //////////////////////////////////////////////////////////////////////
    // class UtilLogger
    //
    // PLEASE NOTE: The class UtilLogger helps with configuring the LoggerCpp options
    // affecting the Log::Logger object during creation (see LoggerCpp 3rd party project),
    // up to the point where the object is initially created (during the very first call
    // made to getLoggerPtr()). Past that point, no more calls to setLoggerOptions() are
    // allowed, and an exception is thrown (if setLoggerOptions() is called).
    //
    // For other LoggerCpp operations on the Log::Logger object, the getLoggerPtr() method
    // can be used to obtain a shared_ptr<Log::Logger> to the Log::Logger object directly,
    // For example:
    //
    // Util::LoggerSPtr myLoggerCppPtr = myUtilLogger_object.getLoggerOptions();
    //
    // which is the equivalent of:
    //
    // std::shared_ptr<Log::Logger> myLoggerCppPtr = myUtilLogger_object.getLoggerOptions();
    //
    // From that point, simply using this as a reference allows the caller to use all of the
    // many facilities of class Log::Logger (*myLoggerCppPtr).whatever()... directly.
    //
    //////////////////////////////////////////////////////////////////////

    // Shared_ptr to class Log::Logger
    using LoggerSPtr = std::shared_ptr<Log::Logger>;

    struct LoggerOptions
    {
        Log::Log::Level loglevel;
        std::string log_level;
        std::string logChannelName;
        std::string logFilelName;
    };

    class UtilLogger
    {
    public:
        UtilLogger(void);
        UtilLogger(const Util::LoggerOptions& logopt);

    private:
        // None of these methods are allowed
        UtilLogger(const UtilLogger &) = delete;
        UtilLogger &operator=(UtilLogger const &) = delete;
        UtilLogger(UtilLogger &&) = delete;
        UtilLogger &operator=(UtilLogger &&) = delete;

    public:
        const Util::LoggerOptions getLoggerOptions() const                  { return m_runtimeLogOpt; }
        static Util::LoggerOptions& getDefaultLoggerOptions()               { return UtilLogger::m_defaultLogOpt; }
        Util::LoggerOptions& setLoggerOptions(Util::LoggerOptions& logopt);
        std::string getLoggerLevelEnumString(Log::Log::Level llevel)        { return std::string(Log::Log::toString(llevel)); }
        LoggerSPtr getLoggerPtr();

    private:
        std::mutex m_UtilLogger_mutex;
        bool m_NoMoreConfig = false;
        LoggerSPtr sp_Logger;
        static Util::LoggerOptions m_defaultLogOpt;
        Util::LoggerOptions m_runtimeLogOpt;
    };

} // end of namespace Util
