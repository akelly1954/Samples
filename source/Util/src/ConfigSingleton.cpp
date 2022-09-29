
//   TODO:             WORK IN PROGRESS

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

#include <ConfigSingleton.hpp>
#include <JsonCppUtil.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace Config;

Json::Value ConfigSingleton::s_configRoot;
ConfigSingletonShrdPtr ConfigSingleton::sp_Instance;
std::mutex ConfigSingleton::s_mutex;
bool ConfigSingleton::s_enabled = false;
std::string ConfigSingleton::s_jsonfilename;

ConfigSingletonShrdPtr ConfigSingleton::create(const std::string& filename)
{
	if (ConfigSingleton::s_enabled)
	{
		throw std::runtime_error("ConfigSingleton object cannot be ::create()d more than once");
	}
	std::lock_guard<std::mutex> lock(s_mutex);

	ConfigSingleton::sp_Instance = std::shared_ptr<ConfigSingleton>(new ConfigSingleton(filename));

	if (!ConfigSingleton::initialize())
	{
	}
	ConfigSingleton::s_enabled = true;
	return ConfigSingleton::sp_Instance;
}

ConfigSingletonShrdPtr ConfigSingleton::get_shared_ptr()
{
	std::lock_guard<std::mutex> lock(ConfigSingleton::s_mutex);
	return this->shared_from_this();
}



ConfigSingletonShrdPtr ConfigSingleton::instance()
{
	return ConfigSingleton::sp_Instance;
}


bool ConfigSingleton::initialize(void)
{
	// TODO:  Have to continue developing this
	UtilJsonCpp::checkjsonsyntax(std::cout, s_jsonfilename);

	Json::Reader reader;
	std::ifstream cfgfile(s_jsonfilename);
	if (!cfgfile.is_open())
	{
		// JsonCpp does not check this, but will fail with a syntax error on the first read
		std::cout << "\nERROR: Could not find json file " << s_jsonfilename << ".  Exiting...\n" << std::endl;
		cfgfile.close();
		return 1;
	}

	cfgfile >> Config::ConfigSingleton::s_configRoot;
	cfgfile.close();
	return true;
}







