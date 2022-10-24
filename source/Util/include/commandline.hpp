#pragma once

#include <Utility.hpp>
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <assert.h>

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
// Command line parsing: class CommandLine declaration is further down.
// 

    enum ParameterStatus
    {
        FlagNotProvided = 0,                // No flag, and therefore no parameter
        FlagProvidedWithEmptyParameter,     // Flag is there but empty or no parameter
        FlagPresentParameterPresent,        // Flag is there, and parameter is there
    };

    using Command_Map = std::map<std::string,std::string>;
    using StringVector = std::vector<std::string>;

    // CommandLine methods:

    ////////////////////////////////////////////////////////////
    // class CommandLine definition
    ////////////////////////////////////////////////////////////

    class CommandLine
    {
    private:
        CommandLine() = default;
    public:
        // hopefully a decent "usage" string is supplied in place of the default one constructed below.
        CommandLine(int argc, const char *argv[], const StringVector& allowedFlags) :
                m_argc(argc),
                m_allowedFlags(allowedFlags)
        {
            // put together our private list of command line members
            for (auto i = 0; i < m_argc; i++)
            {
                m_strArgv.push_back(argv[i]);
            }

            parseCommandLine();
        }

        bool isError(void)                          { return m_isError; };

        std::string getErrorString(void)            { return m_errString; };

        bool isHelp(void)                           { return m_isHelp; };

        // Exposed interface for getting flags and their values.  Uses private
        // get_param_value() (see specialized templates below) for the complete
        // list of supported types.
        template <typename T> Util::ParameterStatus get_template_arg(std::string flag, T& var);

    private:
        // All specialized members assign the appropriate value (converted from the string command line
        // parameter) to the referenced & var parameter.
        template <typename T> Util::ParameterStatus get_param_value(std::string data, T& var);
        void parseCommandLine();
        void setError(std::string errstr)           { m_isError = true; m_errString = errstr; };

    private:
        int m_argc = 0;
        StringVector m_strArgv;
        std::string m_errString;
        bool    m_isError = false;
        bool    m_isHelp = false;
        std::vector<std::string> m_allowedFlags;
        Command_Map m_cmdMap;
    };


    template <typename T>
    inline Util::ParameterStatus CommandLine::get_template_arg(std::string flag, T& var)
    {
        auto it = this->m_cmdMap.find(flag);
        if (it == this->m_cmdMap.end()) return Util::ParameterStatus::FlagNotProvided;

        // it->second is a std::string
        if (Utility::trim(it->second) == "")
        {
            return Util::ParameterStatus::FlagProvidedWithEmptyParameter;
        }

        auto ret = CommandLine::get_param_value<T>(it->second, var);
        assert (ret == Util::ParameterStatus::FlagPresentParameterPresent);
        return ret;
    }

    template <typename T>
    inline Util::ParameterStatus CommandLine::get_param_value(std::string data, T& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        return get_param_value<T>(data, var);
    }


    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<unsigned short>(std::string data, unsigned short& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = static_cast<unsigned short>(strtoul(data.c_str(), NULL, 10) & 0xFFFF);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<bool>(std::string data, bool& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = static_cast<bool>((strtol(data.c_str(), NULL, 10) == 0)? false : true);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<int>(std::string data, int& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = static_cast<int>(strtol(data.c_str(), NULL, 10));
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<long>(std::string data, long& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = static_cast<long>(strtol(data.c_str(), NULL, 10));
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<unsigned long>(std::string data, unsigned long& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = strtoul(data.c_str(), NULL, 10);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<long long>(std::string data, long long& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = static_cast<long long>(strtoll(data.c_str(), NULL, 10));
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<std::string>(std::string data, std::string& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = data;
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<float>(std::string data, float& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = strtof(data.c_str(), NULL);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<double>(std::string data, double& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = strtod(data.c_str(), NULL);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    template <>
    inline Util::ParameterStatus CommandLine::get_param_value<long double>(std::string data, long double& var)
    {
        if (data.length() == 0) return Util::ParameterStatus::FlagNotProvided;
        var = strtold(data.c_str(), NULL);
        return Util::ParameterStatus::FlagPresentParameterPresent;
    }

    ////////////////////////////////////////////////////////////
    // End of class CommandLine definition
    ////////////////////////////////////////////////////////////

} // namespace Util

