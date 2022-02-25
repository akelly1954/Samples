#pragma once

#include <LoggerCpp/LoggerCpp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <utility>
#include <stdint.h>
#include <assert.h>

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

	// Options for NtwkUtilBufferSize
	static const size_t NtwkUtilNanoBufferSize = 64;
	static const size_t NtwkUtilMicroBufferSize = 128;
	static const size_t NtwkUtilTinyBufferSize = 256;
	static const size_t NtwkUtilSmallBufferSize = 1024;
	static const size_t NtwkUtilRegularBufferSize = 4096;
	static const size_t NtwkUtilLargeBufferSize = 8192;

	// NtwkUtilBufferSize - is the fixed size of the std::array<> - baked into the program -
	// can't change (like a #define'd constant can't change at runtime...). The std::array<>
	// object HAS to be built with a fixed size BY DEFINITION.  We can pretend it's not (like
	// we do here) but it is what it is.  At the very least, if the size is not presented as a
	// #define'd constant, then the variable used for the size has to be known to the
	// compiler at compile time (and should therefore be a const which is assigned a value at
	// compile time).
	//
	// NOTE: The fixed std::aray<> size is set here, and can/should be
	// 		 changed right there - it will apply to all objects in NtwkUtil.*
	static const size_t NtwkUtilBufferSize = EnetUtil::NtwkUtilNanoBufferSize;
	//////   OR - whatever - static const size_t NtwkUtilBufferSize = EnetUtil::NtwkUtilRegularBufferSize;

	// Options for port numbers for listening and connecting to:
	// Do not use directly.  See NtwkUtilBufferSize below.
	static const uint16_t base_simple_server_port_number =  57314;

	// This is the real port number. Both connections and listening use this variable.
	// Make sure to rebuild all clients when connecting to a server using this if it changes.
	static const uint16_t simple_server_port_number =
			EnetUtil::base_simple_server_port_number+EnetUtil::NtwkUtilBufferSize;

	// This is the actual type of the data being handled
	typedef std::array<uint8_t,EnetUtil::NtwkUtilBufferSize> arrayUint8;

	class NtwkUtil
	{
	public:

		static int enet_send(Log::Logger& logger,
								int fd,
								EnetUtil::arrayUint8 & array_element_buffer,
								int flag = MSG_NOSIGNAL);

		static int enet_receive(Log::Logger& logger,
								int fd,
								EnetUtil::arrayUint8 & array_element_buffer,	// data and length
								size_t requestsize);

		static std::recursive_mutex m_recursive_mutex;

		// The ip address and port number are used to set the proper values
		// in the empty address structure for all the utilities used in EnetUtil.
		// If the listen address (e.g. "192.168.0.102") is NULL, the INADDR_ANY value (0)
		// will be used in the address structure.
		static bool setup_listen_addr_in ( std::string listen_ip_address, 		// in
									uint16_t socket_port_number, 		// in
									struct ::sockaddr *empty_addr);  	// out

		// Returns socket file descriptor, or -1 on error.
		// We use the logger so that we can capture errno as early as possible
		// after a system call.
		static int server_listen(Log::Logger& logger,
						  struct ::sockaddr *address_struct,
						  int backlog_size);

		// Accepts a single connection request.  It is expected to be called
		// from within a loop.  For each loop cycle, the caller would
		// then start a thread to deal with the request right after this call,
		// if and only if the file descriptor is a valid positive file descriptor
		// from a successful accept() system call, and the sockaddr_in structure
		// is valid.
		static int server_accept(Log::Logger& logger,
						  int listen_socket_fd,
						  struct ::sockaddr *address_struct,
						  int retries = 3);
	};  // end of class NtwkUtil

// END OF SOCKET UTILITIES

} // namespace EnetUtil

