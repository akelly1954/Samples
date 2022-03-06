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

namespace EnetUtil
{

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

// NOTE: The fixed std::aray<> size is set here, and can/should be
//          changed only right here - it will apply to all objects in NtwkUtil.*
//
// If you change a single std::array<> size associated with the various fixed array objects
// in the .cpp sources for example, you are guaranteed a few dozen build error, because... C++.
// And you would deserve it. :-)

// TODO: Note: The fixed_array and related objects are currently specialized to the data
// object being a single byte size.  This needs to be worked in the code to generalize it.

static const size_t NtwkUtilBufferSize = NtwkUtilSmallBufferSize;
///////  Use:       static const size_t NtwkUtilBufferSize = NtwkUtilLargeBufferSize;
//////   Or...      static const size_t NtwkUtilBufferSize = NtwkUtilRegularBufferSize;
//////      or...   static const size_t NtwkUtilBufferSize = NtwkUtilNanoBufferSize;
//////      or...   static const size_t NtwkUtilBufferSize = NtwkUtilSmallBufferSize;
//////     or...

// Options for port numbers for listening and connecting to:
// Do not use directly.  See NtwkUtilBufferSize below.
static const uint16_t base_simple_server_port_number =  57314;

// This is the real port number. Both connections and listening use this variable.
// Make sure to rebuild all clients when connecting to a server using this if it changes.
static const uint16_t simple_server_port_number =
        base_simple_server_port_number+NtwkUtilBufferSize;

// This is the actual type of the data being handled
typedef std::array<uint8_t,NtwkUtilBufferSize> arrayUint8;

// Convenient packaging for external interfaces. The size_t element is
// the number of valid elements in the std::array<> - not the size of the array.
typedef std::pair<size_t,arrayUint8> datapairUint8_t;

class NtwkUtil
{
public:

    static std::recursive_mutex m_recursive_mutex;

    // The ip address and port number are used to set the proper values
    // in the empty address structure for all the utilities used in EnetUtil.
    // If the listen address (e.g. "192.168.0.102") is NULL, the INADDR_ANY value (0)
    // will be used in the address structure.
    static bool setup_sockaddr_in ( std::string listen_ip_address,     // in
                                    uint16_t socket_port_number,    // in
                                    struct ::sockaddr *empty_addr); // out

    // For non-listening processes (setsockopt(), bind() etc are not needed).
    // Returns socket file descriptor, or -1 on error.
    // We use the logger because we want to capture errno as early as possible
    // after an important system call(socket(), listen(), bind(), etc) and log it.
    static int client_socket_connect(Log::Logger& logger, struct ::sockaddr *address);

    // Returns socket file descriptor, or -1 on error.
    // We use the logger because we want to capture errno as early as possible
    // after an important system call(socket(), listen(), bind(), etc) and log it.
    static int server_listen(Log::Logger& logger,
                      struct ::sockaddr *address_struct,
                      int backlog_size);

    // Accepts a single connection request.  It is expected to be called
    // from within a loop.  For each loop cycle, the caller would
    // then start a thread to deal with the request right after this call,
    // if and only if the file descriptor is a valid positive file descriptor
    // from a successful accept() system call, and the sockaddr_in structure
    // is valid.
    static int server_accept( Log::Logger& logger,
                              int listen_socket_fd,
                              struct ::sockaddr *address_struct,
                              int retries = 3);

    static int enet_send (Log::Logger& logger,
                          int fd,
                          arrayUint8 & array_element_buffer,
                          size_t actual_size,
                          int flag = MSG_NOSIGNAL);

    static int enet_receive(Log::Logger& logger,
                            int fd,
                            arrayUint8 & array_element_buffer,
                            // requestsize can be smaller than the array<>::size()
                            size_t requestsize);

    // Get a string message from a remote network connection.
	// retstring is an existing std::string - contents overwritten
	// socket_fd - open connection to the remote system
    // Returns true if successful - restring contains the message from the remote connection.
    // Returns false if failure - retstring contains text about the error.
	static bool get_ntwk_message(Log::Logger& logger, int socket_fd, std::string& retstring);

	// Send the string message to remote connection socket_fd (already open).
	static bool send_ntwk_message(Log::Logger& logger, int socket_fd, std::string& message);

};  // end of class NtwkUtil

}  // end of namespace EnetUtil


