#include "Utility.hpp"
#include "EnetUtil.hpp"
#include "NtwkUtil.hpp"
#include "NtwkConnect.hpp"
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

using namespace EnetUtil;

void writeData(Log::Logger& logger, std::pair<int,arrayUint8>& readypair)
{
	int numread = 0;

	numread = readypair.first;
	arrayUint8& recdarray = readypair.second;

	if (numread > 0)
	{
		char buffer[numread+1];
		void *from = recdarray.data();
		strncpy(buffer, static_cast<const char *>(from), numread);
		buffer[numread] = '\0';
		fwrite(buffer, numread, 1, stdout);
	}
}

const char *logChannelName = "main_ntwk_util";

int main(int argc, char *argv[])
{
    Log::Config::Vector configList;
    // Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, "main_ntwk_util_log.txt", true, true);
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, "", true, false);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

    size_t nread = 0;

	arrayUint8 ibuffer;
	std::pair<int,arrayUint8> pairdata(0,ibuffer);

	while ((nread = NtwkUtil::enetReceive(logger,0,ibuffer,ibuffer.size())) > 0)
	{
		std::pair<int,arrayUint8> readypair(nread, ibuffer);
		writeData(logger, readypair);
	}

	// Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return 0;
}

