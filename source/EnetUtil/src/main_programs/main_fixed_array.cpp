#include "Utility.hpp"
#include "EnetUtil.hpp"
#include "NtwkUtil.hpp"
#include "NtwkConnect.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include <iostream>
#include <vector>
// #include <stdint.h>

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

using namespace EnetUtil;

int main(int argc, char *argv[])
{

    Log::Config::Vector configList;
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, "");
	Log::Config::addOutput(configList, "OutputConsole");

    // Create a Logger object, using a "main_condition_data_log" Channel
    Log::Logger logger("main_fixed_array_log");

    try
    {
        // Configure the Log Manager (create the Output objects)
        Log::Manager::configure(configList);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what();
    }

	std::shared_ptr<fixed_size_array<uint8_t,20>> sp1 = fixed_size_array<uint8_t,20>::create();
	logger.notice() << "----------------- First:";
	logger.notice() << "sp1 use count = " << sp1.use_count();

	std::shared_ptr<fixed_size_array<uint8_t,20>> sp11 = sp1->get_shared_ptr();
	logger.notice() << "----------------- Second:";
	logger.notice() << "sp1 use count = " << sp1.use_count();
	logger.notice() << "sp11 use count = " << sp11.use_count();

	std::shared_ptr<fixed_size_array<uint8_t,20>> sp2 = fixed_size_array<uint8_t,20>::create(*sp1);
	logger.notice() << "----------------- Third:";
	logger.notice() << "sp1 use count = " << sp1.use_count();
	logger.notice() << "sp11 use count = " << sp11.use_count();
	logger.notice() << "sp2 use count = " << sp2.use_count();

	std::shared_ptr<fixed_size_array<uint8_t,20>> sp21 = sp2->get_shared_ptr();
	logger.notice() << "----------------- Fourth:";
	logger.notice() << "sp1 use count = " << sp1.use_count();
	logger.notice() << "sp11 use count = " << sp11.use_count();
	logger.notice() << "sp2 use count = " << sp2.use_count();
	logger.notice() << "sp21 use count = " << sp21.use_count();

	return 0;
}


