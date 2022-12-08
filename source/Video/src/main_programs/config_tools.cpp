
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

#include <config_tools.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <iostream>

bool Config::setup_config_singleton(std::string& result_string,
                                    std::string& ConfigOutputString,
                                    std::vector<std::string>& delayedLinesForLogger)
{
    std::stringstream ostrm;
    try {
        /////////////////
        // Set up config
        /////////////////

        // The assignment is unnecessary - It's here to silence g++ warnings
        // (about discarding the return value)...
        auto thesp = Config::ConfigSingleton::create(Video::vcGlobals::config_file_name, ostrm);

        // At this point the json root node has been set up - after parsing, checking syntax, etc.
        // If ANY errors are encountered along the way, they will be catch()ed below and the
        // program aborted.
        //
        // The root node can be accessed by reference with
        //
        //              Json::Value& ConfigSingleton::instance()->JsonRoot();
        //

    } catch (const std::exception& e) {
        ostrm << "\nConfig singleton setup result: ERROR: Exception while trying to create config singleton: \n    " << e.what() << "\n";
        result_string += ostrm.str(); // The caller can display result_string with cerr
        return false;
    }

    // We will log this as soon as the logger is configured and operational.
    std::string parse_output = std::string("\n\nParsed JSON nodes:") + result_string;

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(parse_output);

    std::stringstream configstrm;
    configstrm << "\n\nParsed JSON file " << Video::vcGlobals::config_file_name << " successfully.  Contents: \n\n"
                      << Config::ConfigSingleton::instance()->JsonRoot() << "\n";

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(configstrm.str());

    // Update the internal global structure with valuses from the JSON file
    try {

        std::stringstream strm;
        if (! Video::updateInternalConfigsWithJsonValues(strm, Config::ConfigSingleton::instance()->JsonRoot()))
        {
            result_string = "Config singleton setup result: ERROR while accessing json values read from ";
            result_string += Video::vcGlobals::config_file_name;
            result_string += ":\n";
            result_string += strm.str();
            result_string += "\n";
            return false;
        }

        ConfigOutputString = strm.str();

        // Everything in the vector will be written to the log file
        // as soon as the logger is initialized.
        delayedLinesForLogger.push_back(ConfigOutputString);
    }
    catch (const std::exception& e)
    {
        result_string = "Config singleton setup result: ERROR: Exception caught while accessing json values read from ";
        result_string += Video::vcGlobals::config_file_name;
        result_string += ":\n";
        result_string += e.what();
        result_string += "\n";
        return false;
    }

    return true;
}














