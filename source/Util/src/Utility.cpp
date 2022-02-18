
#include <stdarg.h>
#include <string>
#include <string.h>
#include <cstdarg>
#include <cstring>
#include <chrono>
#include <sstream>
#include <ctime>
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <iostream>
#include <algorithm>
#include "Utility.hpp"

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


//
// Initializes options in LoggerCpp. Should be run from the main thread.
//
// This is a bit of a hack.  If LoggerCPP persists in these projects,
// some of the hoaky-ness needs to be cleaned up.
void Utility::initializeLogManager( Log::Config::Vector& configList,
									Log::Log::Level loglevel,
									const std::string& logfilename,
									bool enableConsoleOutput,
									bool enableFileOutput)
{
    Log::Manager::setDefaultLevel(loglevel);

    // Configure the Output objects

    // Enforce either console or file output
    if (enableConsoleOutput || (enableConsoleOutput == false && enableFileOutput == false))
    {
    	Log::Config::addOutput(configList, "OutputConsole");
    }

    if (enableFileOutput > 0)
    {
        Log::Config::addOutput(configList, "OutputFile");
        Log::Config::setOption(configList, "filename",          logfilename.c_str());
    }

    // NO ROTATION OF LOG FILES FOR THIS PROGRAM
    // Log::Config::setOption(configList, "filename_old",      "main_condition_data_log.old.txt");

    Log::Config::setOption(configList, "max_startup_size",  "0");
    Log::Config::setOption(configList, "max_size",          "1000000");
#ifdef WIN32
    Log::Config::addOutput(configList, "OutputDebug");
#endif

}
void Utility::configureLogManager( Log::Config::Vector& configList, std::string channelName )
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

long Utility::get_UTC_time_as_long()
{
    using namespace std::chrono;
    milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
    return ms.count();
}

std::string Utility::get_UTC_time_as_string(const char *format)
{
    std::time_t now = std::time(0);
    std::tm *now_tm = std::gmtime(&now);
    char buf[128]; // More than we need
    std::strftime(buf, 128, format, now_tm);
    return static_cast<const char *>(buf);
}

// Get a random int from within a range starting at "low", and
// "low" + "range".  If no "low" number is specified, 0 is used.
// For example, get_rand(10,3) gets you a random number between
// 3 and 12 (inclusive).
int Utility::get_rand(int range, int low)
{
    static bool srand_called = false;
    if (srand_called)
    {
        return (rand() % range) + low;
    }

    srand(time(NULL));
    srand_called = true;
    return (rand() % range) + low;
}

bool Utility::string_starts_with(std::string mainStr, std::string toMatch)
{
    // std::string::find returns 0 if toMatch is found at starting
    if (mainStr.find(toMatch) == 0)
        return true;
    else
        return false;
}

std::string Utility::trim(const std::string &str, const std::string &whitespace)
{
    const auto strBegin = str.find_first_not_of(whitespace);
    if (strBegin == std::string::npos)
        return ""; // no content

    const auto strEnd = str.find_last_not_of(whitespace);
    const auto strRange = strEnd - strBegin + 1;

    return str.substr(strBegin, strRange);
}

std::string Utility::stringFormat(const std::string &format, ...)
{
    va_list args;
    va_start(args, format);
    size_t len = std::vsnprintf(NULL, 0, format.c_str(), args);
    va_end(args);
    std::vector<char> vec(len + 1);
    va_start(args, format);
    std::vsnprintf(&vec[0], len + 1, format.c_str(), args);
    va_end(args);
    return &vec[0];
}

std::vector<std::string> Utility::split(std::string strToSplit, char delimeter)
{
    std::stringstream ss(strToSplit);
    std::string item;
    std::vector<std::string> splitStrings;
    while (std::getline(ss, item, delimeter))
    {
        splitStrings.push_back(item);
    }
    return splitStrings;
}

std::vector<std::string> Utility::split_based_on_word(std::string strToSplit, std::string delimeter)
{
    std::vector<std::string> splitStrings;
    int delimeterLength = delimeter.length();
    while (strToSplit.length() > 0)
    {
        std::size_t found = strToSplit.find(delimeter);
        if (found != std::string::npos)
        {
            splitStrings.push_back(strToSplit.substr(0, found));
            strToSplit.erase(0, found + delimeterLength );
        }
        else
            break;
    }
    return splitStrings;
}

std::string Utility::replace_all(   // Replace all occurences
        const std::string & str ,   // in haystack
        const std::string & find ,  // of needle
        const std::string & replace // with replacement
    )
{
    using namespace std;
    string result;
    size_t find_len = find.size();
    size_t pos,from=0;
    while ( string::npos != ( pos=str.find(find,from) ) ) {
        result.append( str, from, pos-from );
        result.append( replace );
        from = pos + find_len;
    }
    result.append( str, from , string::npos );
    return result;
}

// Converts all chars in str parameter to uppercase (modifies string parameter)
void Utility::to_upper(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c){ return std::toupper(c); });
}

std::string Utility::to_upper(const std::string &str)
{
    std::string ts(str);
    Utility::to_upper(ts);
    return ts;
}

// Converts all chars in str parameter to lowercase (modifies string parameter)
void Utility::to_lower(std::string &str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c){ return std::tolower(c); });
}

std::string Utility::to_lower(const std::string &str)
{
    std::string ts(str);
    Utility::to_lower(ts);
    return ts;
}

std::string Utility::get_errno_message(int errnum)
{
    char buf[1024];

    char *retbuf = strerror_r(errnum, buf, sizeof(buf));
    std::string result = "Errno " + std::to_string(errnum) + ": " + const_cast<const char *>(retbuf);
    return result;
}


