#pragma once

#include "circular_buffer.hpp"
#include <LoggerCpp/LoggerCpp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

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

// The ip address and port number are used to set the proper values
// in the empty address structure for all the utilities used in EnetUtil.
// If the listen address (e.g. "192.168.0.102") is NULL, the INADDR_ANY value (0)
// will be used in the address structure.
bool setup_listen_addr_in ( std::string listen_ip_address, 		// in
							uint16_t socket_port_number, 		// in
							struct ::sockaddr *empty_addr);  	// out

// Returns socket file descriptor, or -1 on error.
// We use the logger so that we can capture errno as early as possible
// after a system call.
int server_listen(Log::Logger& logger,
				  struct ::sockaddr *address_struct,
		          int backlog_size);

// Accepts a single connection request.  It is expected to be called
// from within a loop.  For each loop cycle, the caller would
// then start a thread to deal with the request right after this call,
// if and only if the file descriptor is a valid positive file descriptor
// from a successful accept() system call, and the sockaddr_in structure
// is valid.
int server_accept(Log::Logger& logger, int listen_socket_fd, struct ::sockaddr *address_struct);




















    // TODO: This function is a bit of a hack -- currently using a pipe from an
    // "ip a" process to get its information.  Needs a bit of work, but still useful
    // if used once initially at runtime. Requires the linux ip utility.

    // Used to get a list of MAC addresses belonging to this system which are
    // qualified as "link/ether" in the "ip a" output.
    // Returns empty vector if not found or any errors occured
    std::vector<std::string> get_all_self_mac_addresses();

} // namespace EnetUtil

