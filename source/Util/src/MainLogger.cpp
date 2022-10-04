#include <Utility.hpp>
#include <MainLogger.hpp>
#include <iostream>
#include <stdexcept>

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

using namespace Util;

std::string MainLogger::logChannelName = "application";
std::string MainLogger::logFilelName = logChannelName + "_log.txt";
Log::Log::Level MainLogger::loglevel = Log::Log::Level::eDebug;

std::string MainLogger::default_log_level = Log::Log::toString(MainLogger::loglevel);
std::string MainLogger::log_level = default_log_level;
std::mutex MainLogger::s_logger_mutex;

void Util::MainLogger::initialize(  Log::Config::Vector& configList,
                                    const std::string& channel_name,
                                    Log::Log::Level level,
                                    MainLogger::ConsoleOutput useConsole,
                                    MainLogger::UseLogFile useLogFile
                                 )
{
    std::lock_guard<std::mutex> lock(s_logger_mutex);

    logChannelName = channel_name;
    logFilelName = logChannelName + "_log.txt";
    loglevel = level;

    MainLogger::initializeLogManager(   configList,
                                        loglevel,
                                        logFilelName,
                                        useConsole,
                                        useLogFile);
    MainLogger::configureLogManager( configList, logChannelName );
}

//
// Initializes options in LoggerCpp. Should be run from the main thread.
//
// This is a bit of a hack.  If LoggerCPP persists in these projects,
// some of the hoaky-ness needs to be cleaned up.
void Util::MainLogger::initializeLogManager( Log::Config::Vector& configList,
                                    Log::Log::Level loglevel,
                                    const std::string& logfilename,
                                    MainLogger::ConsoleOutput useConsole,
                                    MainLogger::UseLogFile useLogFile)
{
    Log::Manager::setDefaultLevel(loglevel);

    // Configure the Output objects

    // Enforce either console or file output
    if (useConsole == MainLogger::enableConsole ||
            (useConsole == MainLogger::disableConsole && useLogFile == MainLogger::disableLogFile))
    {
        Log::Config::addOutput(configList, "OutputConsole");
    }

    if (useLogFile == MainLogger::enableLogFile)
    {
        Log::Config::addOutput(configList, "OutputFile");
        Log::Config::setOption(configList, "filename",          logfilename.c_str());
        std::string oldlogfilename = std::string("old.")+logfilename;
        Log::Config::setOption(configList, "filename_old",      oldlogfilename.c_str());
        std::cerr << "Log file: " << logfilename.c_str() << std::endl;
    }

    Log::Config::setOption(configList, "max_startup_size",  "0");
    Log::Config::setOption(configList, "max_size",          "100000000");
#ifdef WIN32
    Log::Config::addOutput(configList, "OutputDebug");
#endif

}

void Util::MainLogger::configureLogManager( Log::Config::Vector& configList, std::string channelName )
{
    // Create a Logger object, using the parameter Channel
    Log::Logger logger(channelName.c_str());

    try
    {
        // Configure the Log Manager (create the Output objects)
        Log::Manager::configure(configList);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }
}

//////////////////////////////////////////////////////////////////////
// class UtilLogger members - please see large comment in MainLogger.hpp.
//////////////////////////////////////////////////////////////////////

// static

Util::LoggerOptions UtilLogger::m_defaultLogOpt = {
                                        // loglevel (above)
                                        Log::Log::Level::eInfo,

                                        // log_level (above)
                                        std::string((Log::Log::toString(Log::Log::Level::eInfo))),

                                        // logChannelName (above)
                                        std::string("app"),

                                        // logFileName (above)
                                        m_defaultLogOpt.logChannelName + "_log.txt"
                                    };
// methods

UtilLogger::UtilLogger(const Util::LoggerOptions& logopt)
{
    // use your own object to initialise the logger
    m_runtimeLogOpt = logopt;
}

UtilLogger::UtilLogger()
{
    // use the default settings for the logger
    m_runtimeLogOpt = UtilLogger::m_defaultLogOpt;
}

LoggerSPtr UtilLogger::getLoggerPtr()
{
    if (sp_Logger != nullptr)
    {
        return sp_Logger;
    }

    std::lock_guard<std::mutex> lock(m_UtilLogger_mutex);

    // the first check above wasn't locked (for speed). So we check again
    // now that this code path is locked.
    if (sp_Logger != nullptr)
    {
        return sp_Logger;
    }

    // Set up the new logger
    Log::Config::Vector configList;
    Util::MainLogger::initializeLogManager( configList,
                                            m_runtimeLogOpt.loglevel,
                                            m_runtimeLogOpt.logFilelName,
                                            Util::MainLogger::disableConsole,
                                            Util::MainLogger::enableLogFile);

    Util::MainLogger::configureLogManager( configList, m_runtimeLogOpt.logChannelName);

    sp_Logger = std::make_shared<Log::Logger>(m_runtimeLogOpt.logChannelName.c_str());
    m_NoMoreConfig = true;

    return sp_Logger;
}

Util::LoggerOptions& UtilLogger::setLoggerOptions(Util::LoggerOptions& logopt)
{
    if (m_NoMoreConfig)
    {
        throw std::runtime_error("Cannot set additional options after Log::Logger object creation.");
    }
    m_runtimeLogOpt = logopt;
    m_runtimeLogOpt.log_level = getLoggerLevelEnumString(m_runtimeLogOpt.loglevel);
    return logopt;
}

