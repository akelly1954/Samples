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
// In your your code, call getCLMap() with the command line parameters (arc, argv).
// The function will return a std::map<string,string>, organized by flags.
// 
// By definition, a flag is made up of three characters: A dash ('-') followed by two letters.
// This is not negotiable at this point.
// 
// What follows is a code snippet taken from main_client_for_basic_server.cpp, showing how to
// get information about the command line.  The two cases covered are: a mandatory flag case,
// followed by an optional flag that has a default:
// 
// FIRST CASE:
//
// cmdmap is the std::map<std::string, std::sting> object returned by getCLMap().
// input_filename is an std::string which will get the specified file name.
// 
// this flag (-fn) and an existing readable regular file name are MANDATORY
// switch(getArg(cmdmap, "-fn", input_filename))
// {
//   case Util::ParameterStatus::FlagNotProvided:
//         strm << "ERROR: the \"-fn\" flag is missing. Specifying input file name with the -fn flag is mandatory." << std::endl;
//         return false;
//     case Util::ParameterStatus::FlagPresentParameterPresent:
//         strm << "-fn flag provided. Using " << input_filename << std::endl;
//         break;
//     case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
//         strm << "ERROR: -fn flag is missing its parameter." << std::endl;
//         return false;
//     default:
//         assert (argc == -668);   // Bug encountered. Will cause abnormal termination
// }
//
// SECOND CASE:
//
// cmdmap is the std::map<std::string, std::sting> object returned by getCLMap().
// connection_port_number is a uint16_t (unsigned short) which will be assigned the port number
// from the command line if it is specified. (unsigned short is the type for port numbers used in
// system calls and ioctl calls used by IPV4 in linux). It holds the default port number, and that
// will persist if the -pn parameter is not specified on the command line.
// 
// switch(getArg(cmdmap, "-pn", connection_port_number))
// {
//     case Util::ParameterStatus::FlagNotProvided:
//         strm << "-pn flag not provided. Using default " << connection_port_number << std::endl;
//         break;
//     case Util::ParameterStatus::FlagPresentParameterPresent:
//         strm << "-pn flag provided. Using " << connection_port_number << std::endl;
//         break;
//     case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
//         strm << "ERROR: -pn flag is missing its parameter." << std::endl;
//         return false;
//     default:
//         assert (argc == -667);   // Bug encountered. Will cause abnormal termination
// }
//
// This manner of dealing with the command line is VERY verbose.  However it is also VERY clear and
// straight forward.  MHO.
// 

std::map<std::string,std::string> getCLMap(int argc, char *argv[]);

// Command line parsing
enum ParameterStatus
{
    FlagNotProvided = 0,                // No flag, and therefore no parameter
    FlagProvidedWithEmptyParameter,     // Flag is there but empty or no parameter
    FlagPresentParameterPresent,        // Flag is there, and parameter is there
};

// Generic template function definition.
// See the specialized versions below.
template <typename T>
bool get_param_value(std::string data, T& var)
{
    if (data.length() == 0) return false;
    var = data;
    return true;
}

template <>
bool get_param_value<unsigned short>(std::string data, unsigned short& var)
{
    if (data.length() == 0) return false;
    var = static_cast<unsigned short>(strtoul(data.c_str(), NULL, 10) & 0xFFFF);
    return true;
}

template <>
bool get_param_value<int>(std::string data, int& var)
{
    if (data.length() == 0) return false;
    var = static_cast<unsigned short>(strtol(data.c_str(), NULL, 10));
    return true;
}

template <>
bool get_param_value<long>(std::string data, long& var)
{
    if (data.length() == 0) return false;
    var = static_cast<unsigned short>(strtol(data.c_str(), NULL, 10));
    return true;
}

template <>
bool get_param_value<unsigned long>(std::string data, unsigned long& var)
{
    if (data.length() == 0) return false;
    var = strtoul(data.c_str(), NULL, 10);
    return true;
}

template <>
bool get_param_value<long long>(std::string data, long long& var)
{
    if (data.length() == 0) return false;
    var = static_cast<unsigned short>(strtoll(data.c_str(), NULL, 10));
    return true;
}

template <>
bool get_param_value<std::string>(std::string data, std::string& var)
{
    if (data.length() == 0) return false;
    var = data;
    return true;
}

template <>
bool get_param_value<float>(std::string data, float& var)
{
    if (data.length() == 0) return false;
    var = strtof(data.c_str(), NULL);
    return true;
}

template <>
bool get_param_value<double>(std::string data, double& var)
{
    if (data.length() == 0) return false;
    var = strtod(data.c_str(), NULL);
    return true;
}

template <>
bool get_param_value<long double>(std::string data, long double& var)
{
    if (data.length() == 0) return false;
    var = strtold(data.c_str(), NULL);
    return true;
}

// Generic template function definition.
// There are no specialized versions at this time, because of the use of get_param_value<T>().
template <typename T>
Util::ParameterStatus get_template_arg(const std::map<std::string,std::string>& cmdmap, std::string flag, T& var)
{
    auto it = cmdmap.find(flag);
    if (it == cmdmap.end()) return Util::ParameterStatus::FlagNotProvided;

    // Not checking for string length because get_param_value<T>() does.
    return Util::get_param_value<T>(it->second, var) ?
            Util::ParameterStatus::FlagPresentParameterPresent :
            Util::ParameterStatus::FlagProvidedWithEmptyParameter;
}

// Convenience overloaded functions meant for the delicate user who
// does not wish to deal with templates? Truth is - keeping these around
// for backward compatibility.
//
// NOTE: The return value of all getArg() functions changed from bool to an enum value.  The c++ compiler
// will allow the assignment of this return value to a bool.  "Just so happens" that the value of the enum that
// signifies error is 0 - same as false. That's why you don't have to change anything in the code if it still
// accepts the value of getArg() as a bool.
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned short& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned long& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long long& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, float& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var);
Util::ParameterStatus getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long double& var);

} // namespace Util

