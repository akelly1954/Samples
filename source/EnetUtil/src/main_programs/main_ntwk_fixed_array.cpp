
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

#include <Utility.hpp>
#include <MainLogger.hpp>
#include "NtwkUtil.hpp"
#include "NtwkFixedArray.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include <iostream>
#include <vector>

using namespace EnetUtil;

void initialize(Util::LoggerSPtr loggerp, uint8_t val, std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp)
{
    for (size_t i = 0; i < NtwkUtilBufferSize; i++)
    {
        if (! sp->set_element(i, val+i) )
        {
            loggerp->error() << "Could not access array member "  << std::to_string(i) << " for set_element to " << std::to_string(val+i);
        }
    }
}

void print(Util::LoggerSPtr loggerp, std::string label, std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp)
{
    loggerp->notice() << " ";    // Empty line
    // Decreasing the use_count by 1, to account for the shared_ptr parameter to this function
    loggerp->notice() << label << ", use count = " << std::to_string(sp.use_count()-1) << "  ------------------------";
    loggerp->notice() << " ";    // Empty line

    for (size_t i = 0; i < NtwkUtilBufferSize; i++)
    {
        uint8_t val;
        if (sp->get_element(i, val))
        {
            loggerp->notice() << "array[" << std::to_string(i) << "] = " << std::to_string(val);
        }
    }
}

// All threads, including main, output to this channel
const char *logChannelName = "main_fixed_array";
Log::Log::Level loglevel = Log::Log::Level::eNotice;

int main(int argc, char *argv[])
{
    using namespace Util;

    /////////////////
    // Set up logger
    /////////////////
    Util::LoggerOptions localopt = Util::UtilLogger::setLocalLoggerOptions(
                                                        logChannelName,
                                                        loglevel,
                                                        Util::MainLogger::enableConsole,
                                                        Util::MainLogger::disableLogFile
                                                    );
    Util::UtilLogger::create(localopt);
    LoggerSPtr loggerp = UtilLogger::getLoggerPtr();

    std::cout << "Log level is set to \"" << Log::Log::toString(loglevel) << "\"" << std::endl;


#if 0 // TODO:
    Log::Config::Vector configList;
    MainLogger::initializeLogManager(configList, Log::Log::Level::eNotice, "", Util::MainLogger::enableConsole, Util::MainLogger::disableLogFile);
    MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);
#endif // 0

    try
    {
        std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp1 = fixed_size_array<uint8_t,NtwkUtilBufferSize>::create();
        initialize(loggerp, 1, sp1);
        print(loggerp, "Object sp1 created with ::create()", sp1);

        std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp11 = sp1->get_shared_ptr();
        initialize(loggerp, 11, sp11);

        print(loggerp, "Object sp11 after getting shared_ptr from sp1", sp11);
        print(loggerp, "Checking sp1", sp1);
        loggerp->notice() << "\n-----------------------------------------------------------------------\n";

        std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp2 = fixed_size_array<uint8_t,NtwkUtilBufferSize>::create(*sp1);
        initialize(loggerp, 2, sp2);
        print(loggerp, "Object sp2 after creation using create with copy constructor", sp2);

        std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp21 = sp2->get_shared_ptr();
        initialize(loggerp, 21, sp21);
        print(loggerp, "Object sp21 after getting shared_ptr from sp2", sp21);
        print(loggerp, "Checking sp2", sp2);
        loggerp->notice() << "\n-----------------------------------------------------------------------\n";

        std::shared_ptr<fixed_size_array<uint8_t,NtwkUtilBufferSize>> sp30 = fixed_size_array<uint8_t,NtwkUtilBufferSize>::create(*sp1);
        initialize(loggerp, 30, sp30);
        print(loggerp, "Object sp30 before assignment from sp1", sp30);

        *sp30 = *sp1;

        if (! sp30->set_element(2, 100) )
        {
            loggerp->error() << "Could not access sp30 member 2 for set_element to 100";
        }

        // Change a single array element in sp30.  It should not show in sp1
        print(loggerp, "Object sp30 after assignment from sp1, with a single element change", sp30);
        print(loggerp, "Checking sp1. Element 2 should be unchanged", sp1);
    }
    catch (std::exception& e)
    {
        loggerp->debug() << e.what();
    }
    return 0;
}


