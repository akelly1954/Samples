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

namespace Util {

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

