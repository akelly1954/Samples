#pragma once
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <LoggerCpp/LoggerCpp.h>

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

namespace Util {

	// This is where odds and ends go
	class Utility {

	private:
		// Not allowed:
		Utility(void) = delete;
		Utility(const Utility &) = delete;
		Utility &operator=(Utility const &) = delete;
		Utility(Utility &&) = delete;
		Utility &operator=(Utility &&) = delete;

	public:

        static void initializeLogManager(Log::Config::Vector& configList, Log::Log::Level loglevel, const std::string& logfilename = "log.txt");
		static long get_UTC_time_as_long();
		static std::string get_UTC_time_as_string(const char *format = "%Y-%m-%dT%XZ");

		// Get a random int from within a range starting at "low", and
		// "low" + "range".  If no "low" number is specified, 0 is used.
		// For example, get_rand(10,3) gets you a random number between
		// 3 and 12 (inclusive).
		static int get_rand(int range, int low = 0);

        static std::vector<std::string> split_based_on_word(std::string strToSplit, std::string delimeter);

	    static bool string_starts_with(std::string mainStr, std::string toMatch);
	    static std::string trim(const std::string& str,const std::string& whitespace = " \t");
	    static std::string stringFormat(const std::string& format, ...);
	    static std::vector<std::string> split(std::string strToSplit, char delimeter);

	    // Replace all occurences in ------------ haystack of -------- needle with -------- replacement
	    static std::string replace_all(const std::string &, const std::string &, const std::string &);

	    // Converts all chars in str parameter to uppercase (modifies string parameter)
	    static void to_upper(std::string &str);
	    static std::string to_upper(const std::string &str);

	    // Converts all chars in str parameter to lowercase (modifies string parameter)
	    static void to_lower(std::string &str);
	    static std::string to_lower(const std::string &str);

	    // Called with errno after failure to get error string
	    static std::string get_errno_message(int);
	};
} // namespace Util

