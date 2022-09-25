
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

#include <iostream>
#include <json/json.h>
#include <fstream>
#include <JsonCppUtil.hpp>

int main()
{
    std::string config_file_name = "main_jsoncpp_samplecfg.json";

    // TODO:  Have to continue developing this
	// TODO:  Just for now.
	UtilJsonCpp::checkjsonsyntax(std::cout, config_file_name);

    Json::Reader reader;
    Json::Value cfg_root;
    std::ifstream cfgfile("main_jsoncpp_samplecfg.json");
    if (!cfgfile.is_open())
    {
        // JsonCpp does not check this, but will fail with a syntax error on the first read
        std::cout << "\nERROR: Could not find json file " << config_file_name << ".  Exiting...\n" << std::endl;
        return 1;
    }

    cfgfile >> cfg_root;
    std::cout << "\n" << cfg_root << std::endl;

    return 0;
}       

