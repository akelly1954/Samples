#include "commandline.hpp"

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
std::map<std::string,std::string> Util::getCLMap(int argc, char *argv[])
{
    std::map<std::string,std::string> cmdmap;

    int i = 1;
    do
    {
        if (i >= argc) break;

        std::string currentArg = const_cast<const char *>(argv[i]);
        if (currentArg[0] == '-' && currentArg.length() == 3)
        {
            // This is a new flag
            if (i < argc-1)
            {
                cmdmap[currentArg] = const_cast<const char *>(argv[++i]);
            }
        }
        i++;
    } while (i < argc);
    return cmdmap;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, unsigned short& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = static_cast<unsigned short>(strtoul(it->second.c_str(), NULL, 10) & 0xFFFF);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = strtol(it->second.c_str(), NULL, 10);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = strtol(it->second.c_str(), NULL, 10);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long long& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = strtoll(it->second.c_str(), NULL, 10);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = it->second;
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, float& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
    	var = strtof(it->second.c_str(), NULL);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
    	var = strtod(it->second.c_str(), NULL);
        ret = true;
    }
    return ret;
}

bool Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long double& var)
{
	bool ret = false;
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
    	var = strtold(it->second.c_str(), NULL);
        ret = true;
    }
    return ret;
}
