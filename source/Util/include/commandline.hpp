#pragma once

#include <stdlib.h>
#include <map>
#include <string>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// For example of use, see the main program in main_programs/main_commandline.cpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Util {

std::map<std::string,std::string> getCLMap(int argc, char *argv[]);

bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long long& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, float& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var);
bool getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, long double& var);

} // namespace Util

