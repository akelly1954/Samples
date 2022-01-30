#include "commandline.hpp"

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
