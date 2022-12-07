
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

bool Config::setup_config_singleton(std::string& restring)
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
        ostrm << "\nERROR: Exception while trying to create config singleton: \n    " << e.what() << "\n";
        restring += ostrm.str();
        return false;
    }
    return true;
}

