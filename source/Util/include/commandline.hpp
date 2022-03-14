#pragma once

#include <stdlib.h>
#include <map>
#include <string>

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

//
// For example of use, see the main program in main_programs/main_commandline.cpp
//
// TODO: enhance the main program that tests this, to account for changes in the command line structure.
//

namespace Util {

// 
// Command line parsing:
// 
// In your main(), call getCLMap() with the command line parameters (arc, argv).
// The function will return a std::map<string,string>, organized by flags.
// 
// By definition, a flag is made up of three characters: A dash ('-') followed by two letters.
// This is not negotiable at this point.
// 
// Say a command line looks like this:      "program -f1 param2 -f2 -f3 param3"
// 
// Each pair of map members have:  it->first (std::string) which has the flag, say "-f3",
// and it->second (std::string), which has the value of the parameter for the flag ("param3"), or an
// empty string.
// 
// The empty string could be the result of no paramter having been provided, or because the
// empty string was specified as a parameter ("").
// 
// Try not to use the ::operator[] functionality of std::map. Using this type of construct,
// (*it)["-f1"], will create an empty instance of this flag in the map, with an empty parameter
// string.  Use the various ::find() functions instead.
// 
// The overloaded getArg() functions, below, will return true or false depending on whether
// a specific flag is present on the command line, and will convert the parameter, if it
// exists, to the correct type (int, short, long long, double, etc etc).  It returns true
// regardless of whether the paramter to the flag existed, and will assign the value of the
// parameter if and only if the flag in question had a parameter on the command line.
// 
// So, for example, for the command line above, after getting the std::map using getCLMap(),
// use getArg() like this for "-f2":
// 
//     int defaultParam2 = 33;
//     int cmdlineParam2 = defaultParam2;
// 
//     // cmdmap is the std::map<std::string, std::sting> object returned by getCLMap().
//     if (getArg(cmdmap, "-f2", cmdlineParam2) == true)
//     {
//         // The "-f2" flag was used on the command line
//         // The cmdlineParam2 value will not be modified unless a command line
//         // parameter was present.  One way to deal with that is like this:
//         if (cmdlineParam2 != defaultParam2)
//         {
//             // just use the value at this point - maybe print a nice message
//             // Alternatively, if the parameter is not optional - create an error
//             // condition here (return value?  exception?)
//         }
//     }
//     else
//     {
//         // The "-f2" flag was NOt used on the command line
//         // At this point, you could just use the default:
//         cmdlineParam2 = defaultParam2;
//     }
// 
// etc.
// 
// The source file in ...Sample/source/Video/src/main_programs/main_v4l2_raw_capture.cpp
// Contains a good exampele of how to use this mechanism.

std::map<std::string,std::string> getCLMap(int argc, char *argv[]);

bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned short& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long long& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, float& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long double& var);

} // namespace Util

