#include "commandline.hpp"

std::map<std::string,std::string> Util::getCLMap(int argc, char *argv[])
{
    std::map<std::string,std::string> cmdmap;

    int i = 1;
    do
    {
        if (i >= argc) break;

        std::string currentArg = const_cast<const char *>(argv[i]);
        if (currentArg[0] == '-' && currentArg.length() == 2)
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

void Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var)
{
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = strtol(it->second.c_str(), NULL, 10);
    }
}

void Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var)
{
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
        var = it->second;
    }
}

void Util::getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var)
{
    auto it = cmdmap.find(flag);
    if (it != cmdmap.end() && it->second.length() > 0)
    {
    	var = strtod(it->second.c_str(), NULL);
    }
}

