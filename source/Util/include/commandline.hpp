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

void getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, int& var);

void getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, std::string& var);

void getArg(const std::map<std::string,std::string>& cmdmap, std::string flag, double& var);

} // namespace Util

