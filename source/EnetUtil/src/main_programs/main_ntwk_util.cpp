#include "Utility.hpp"
#include "MainLogger.hpp"
#include "NtwkUtil.hpp"
#include "NtwkFixedArray.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include <condition_data.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <algorithm>

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

// This program is proof-of-concept and test for some of the array<> related objects
// used in this project.  Reads from stdin, and writes to stdout (appearances to
// the contrary).  Redirect stdin from a file, and redirect stdout to a new file.
// Then run "cmp file1 file2" and you should see no differences.  $? should be 0.
// Change the initialized value of NtwkUtilBufferSize in NtwkUtil.hpp, and rebuild.
// Results should be the same.

///////////////////////////////////////////////////////////
// NOTE:
// THIS PROGRAM IS BUILT TO FAIL (testing objects)
///////////////////////////////////////////////////////////

using namespace EnetUtil;

void writeData(Util::LoggerSPtr loggerp, datapairUint8_t& readypair)
{
    if (readypair.first > 0)
    {
        char buffer[readypair.first+1];
        void *from = readypair.second.data();
        strncpy(buffer, static_cast<const char *>(from), readypair.first);
        buffer[readypair.first] = '\0';
        fwrite(buffer, readypair.first, 1, stdout);
    }
}

std::string logChannelName = "ntwk_util";
Log::Log::Level logLevel = Log::Log::Level::eDebug;

int main(int argc, char *argv[])
{
    using namespace Util;

    LoggerOptions localopt = UtilLogger::setLocalLoggerOptions(
                                                    logChannelName,
                                                    logLevel,
                                                    MainLogger::enableConsole,
                                                    MainLogger::disableLogFile
                                                );
    Util::UtilLogger::create(localopt);
    std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();

    arrayUint8 ibuffer;                     // input buffer
    datapairUint8_t pairdata(0,ibuffer);    // pairdata.first will hold the valid number elements in the std::array<>

    int ret = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////
    // The 0 parameter to enet_receive() is stdin - not a socket. This will cause an error.
    ////////////////////////////////////////////////////////////////////////////////////////////
    while (1)
    {
        ret = NtwkUtil::enet_receive(loggerp, 0, pairdata.second, pairdata.second.size());
        if (ret == -1)
        {
            ret = 1;    // ret becomes the return code for main()
            break;
        }
        else if (ret == 0)
        {
            break;
        }
        pairdata.first = ret;
        writeData(loggerp, pairdata);
    }

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return ret;
}

