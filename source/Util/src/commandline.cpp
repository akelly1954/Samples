#include "commandline.hpp"
#include <algorithm>

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

// Currently hard-coded to deal with the first command line argument being
// in the form of -xx.
std::map<std::string,std::string> Util::getCLMap(int argc, const char *argv[], const std::vector<std::string>& allowedFlags)
{
    std::map<std::string,std::string> cmdmap;

    int i = 1;
    do
    {
        if (i >= argc) break;

        std::string currentArg = const_cast<const char *>(argv[i]);

        if (currentArg.length() == 0)
        {
            continue;  // ignore weird empty strings
        }
        else if (std::find(allowedFlags.begin(), allowedFlags.end(), currentArg) != allowedFlags.end())
        {
            // This is a new flag
            if (i < argc-1)
            {
                std::string nextarg = const_cast<const char *>(argv[i+1]);
                if (std::find(allowedFlags.begin(), allowedFlags.end(), nextarg) == allowedFlags.end()) // Is the next arg another flag?
                {
                    cmdmap[currentArg] = nextarg;
                    i++;
                }
                else
                {
                    // This is a flag with no parameter
                    cmdmap[currentArg] = "";  // i is not incremented because no parameter there.
                }
            }
            else
            {
                // This is a flag at the last parameter on the command line.
                // There is no parameter
                cmdmap[currentArg] = "";
                break;
            }
        }
        i++;
    } while (i < argc);
    return cmdmap;
}


Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned short& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, bool& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long long& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, float& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var)
{
    return get_template_arg(cmdmap, flag, var);
}

Util::ParameterStatus Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long double& var)
{
    return get_template_arg(cmdmap, flag, var);
}
