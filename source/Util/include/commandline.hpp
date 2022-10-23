#pragma once

#include <stdlib.h>
#include <map>
#include <vector>
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
// For example of use, see main programs in the Samples project that use command lines, as
// well as main_programs/main_commandline.cpp
//

namespace Util {

// 
// Command line parsing:
// 
using Command_Map = std::map<std::string,std::string>;

class CommandLine
{

};


















Command_Map getCLMap(int argc, const char *argv[], const std::vector<std::string>& allowedFlags);
Command_Map parseSetup(int argc, const char *argv[], const std::vector<std::string>& allowedFlags);
std::string parseGetHelp(Command_Map cmdmap);
std::string parseGetError(Command_Map cmdmap);

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
bool get_param_value<bool>(std::string data, bool& var)
{
    if (data.length() == 0) return false;
    var = static_cast<bool>((strtol(data.c_str(), NULL, 10) == 0)? false : true);
    return true;
}

template <>
bool get_param_value<int>(std::string data, int& var)
{
    if (data.length() == 0) return false;
    var = static_cast<int>(strtol(data.c_str(), NULL, 10));
    return true;
}

template <>
bool get_param_value<long>(std::string data, long& var)
{
    if (data.length() == 0) return false;
    var = static_cast<long>(strtol(data.c_str(), NULL, 10));
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
    var = static_cast<long long>(strtoll(data.c_str(), NULL, 10));
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
Util::ParameterStatus get_template_arg(const Command_Map& cmdmap, std::string flag, T& var)
{
    auto it = cmdmap.find(flag);
    if (it == cmdmap.end()) return Util::ParameterStatus::FlagNotProvided;

    // Not checking for string length because get_param_value<T>() does.
    return Util::get_param_value<T>(it->second, var) ?
            Util::ParameterStatus::FlagPresentParameterPresent :
            Util::ParameterStatus::FlagProvidedWithEmptyParameter;
}

// Convenience overloaded functions meant for the delicate user who
// does not wish to deal with templates.
//
// NOTE: The return value of all getArg() functions changed from bool to an enum value.  The c++ compiler
// will allow the assignment of this return value to a bool.  "Just so happens" that the value of the enum that
// signifies error is 0 - same as false. That's why you don't have to change anything in the code if it still
// accepts the value of getArg() as a bool.
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, unsigned short& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, bool& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, int& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, long& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, unsigned long& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, long long& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, std::string& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, float& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, double& var);
Util::ParameterStatus getArg(const Command_Map& cmdmap, std::string flag, long double& var);

} // namespace Util

