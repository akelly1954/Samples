#pragma once

#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef NOBUILD

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <thread>

#endif // NOBUILD


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

namespace EnetUtil {

	class socket_connection_thread
	{
	public:
		// This member function (static) runs in its own thread
		static void handler (int socket, int threadno, Log::Logger logger);

		static bool write_to_file(  Log::Logger& logger,
									std::shared_ptr<fixed_uint8_array_t> data_sp,
									std::string output_filename);

		static std::string get_seq_num_string(long num);	// utility function

		// This member function (static) runs in the main thread.
		static void start (int socket, int threadno, const char *logChannelName = "socket_connection_thread");

		static void terminate_all_threads()
		{
			std::for_each(s_connection_workers.begin(), s_connection_workers.end(), [](std::thread &t)
			{
				if (t.joinable()) t.join();
			});
		}

		// vector of std::thread objects, each handling its own connection
		// (see handler() function).
		static std::vector<std::thread> s_connection_workers;

	};  // end of class socket_connection_thread

} // end of namespace EnetUtil




