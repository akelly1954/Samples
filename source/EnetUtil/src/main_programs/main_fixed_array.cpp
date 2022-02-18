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

void initialize(Log::Logger& logger, uint8_t val, const std::array<uint8_t,5>* array)
{
	std::array<uint8_t,5> *data = const_cast<std::array<uint8_t,5> *>(array);

	uint8_t ui8 = val+1;
	for (auto it = data->begin(); it != data->end(); ++it)
	{
		*it = ui8;
	}
}

void print(Log::Logger& logger, std::string label, std::shared_ptr<fixed_size_array<uint8_t,5>> sp)
{
	logger.notice() << " ";	// Empty line
	// Decreasing the use_count by 1, to account for the share_ptr parameter to this function
	logger.notice() << label << " use count = " << std::to_string(sp.use_count()-1) << "  ------------------------";
	logger.notice() << " ";	// Empty line

	std::array<uint8_t,5>* array = const_cast<std::array<uint8_t,5> *>(sp->data());
	uint8_t i = 0;
	for (auto it = array->begin(); it != array->end(); ++it, i++)
	{
		logger.notice() << "array[" << std::to_string(i) << "] = " << std::to_string(*it + i);
	}
}

// All threads, including main, output to this channel
const char *logChannelName = "main_fixed_array";

int main(int argc, char *argv[])
{

    Log::Config::Vector configList;
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, "", false, false);
    Util::Utility::configureLogManager( configList, "main_fixed_array_log" );
    Log::Logger logger(logChannelName);

    try
    {
        std::shared_ptr<fixed_size_array<uint8_t,5>> sp1 = fixed_size_array<uint8_t,5>::create();
        initialize(logger, 1, const_cast<std::array<uint8_t,5> *>(sp1->data()));
        print(logger, "Object sp1 created with ::create()", sp1);

        std::shared_ptr<fixed_size_array<uint8_t,5>> sp11 = sp1->get_shared_ptr();
        initialize(logger, 11, const_cast<std::array<uint8_t,5> *>(sp11->data()));

        print(logger, "Object sp2 after getting shared_ptr from sp1", sp11);
        print(logger, "Checking sp1", sp1);
        logger.notice() << "\n-----------------------------------------------------------------------\n";

        std::shared_ptr<fixed_size_array<uint8_t,5>> sp2 = fixed_size_array<uint8_t,5>::create(*sp1);
        initialize(logger, 2, const_cast<std::array<uint8_t,5> *>(sp2->data()));
        print(logger, "Object sp2 after creation using create with copy constructor", sp2);

        std::shared_ptr<fixed_size_array<uint8_t,5>> sp21 = sp2->get_shared_ptr();
        initialize(logger, 21, const_cast<std::array<uint8_t,5> *>(sp21->data()));
        print(logger, "Object sp21 after getting shared_ptr from sp2", sp21);
        print(logger, "Checking sp2", sp2);
        logger.notice() << "\n-----------------------------------------------------------------------\n";

        std::shared_ptr<fixed_size_array<uint8_t,5>> sp30 = fixed_size_array<uint8_t,5>::create(*sp1);
        initialize(logger, 30, const_cast<std::array<uint8_t,5> *>(sp30->data()));
        print(logger, "Object sp30 before assignment from sp1", sp30);

        *(sp30) = *(sp1);

        // TODO: There HAS to be a better way to get to an array element... do it.

    	std::array<uint8_t,5>* array = const_cast<std::array<uint8_t,5> *>(sp30->data());
    	(*array)[2] = 100;

        // Change a single array element in sp30.  It should not show in sp1
        print(logger, "Object sp30 after assignment from sp1, with a single element change", sp30);
        print(logger, "Checking sp1. Element 2 should be unchanged", sp1);

        /////////////////////////////////////////////
        // Intentional Errors
        /////////////////////////////////////////////

        // This will produce a compile error as intended (should use ::create() instead.
        // EnetUtil::fixed_size_array<T, N>::fixed_size_array() [with T = unsigned char; long unsigned int N = 5]’
        //                                                                           is private within this context
        //////		fixed_size_array<uint8_t,5> errorObject;

        // This will produce a compile error as intended (should use ::create() instead.
        // EnetUtil::fixed_size_array<T, N>::fixed_size_array() [with T = unsigned char; long unsigned int N = 5]’
        //                                                                           is private within this context
        //
        /////		std::shared_ptr<fixed_size_array<uint8_t,5>> sp1_error = std::make_shared<fixed_size_array<uint8_t,5>>();
    }
    catch (std::exception& e)
    {
        logger.debug() << e.what();
    }
	return 0;
}


