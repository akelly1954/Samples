#include "Utility.hpp"
#include "EnetUtil.hpp"
#include <iostream>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>


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

// SOCKET UTILITIES

// The ip address and port number are used to set the proper values
// in the empty address structure for all the utilities used in EnetUtil.
// If the listen address (e.g. "192.168.0.102") is NULL, the INADDR_ANY value (0)
// will be used in the address structure.
bool EnetUtil::setup_listen_addr_in(std::string listen_ip_address, 		// in
									uint16_t socket_port_number, 		// in
									struct ::sockaddr *addr)  			// out
{
	struct ::sockaddr_in *empty_addr = (::sockaddr_in *) addr;
	empty_addr->sin_family = AF_INET;

	::in_addr_t x;
	if (listen_ip_address.empty() || listen_ip_address == "INADDR_ANY")
	{
		x = INADDR_ANY;
	}
	else
	{
		x = inet_addr(listen_ip_address.c_str());
	}

	empty_addr->sin_addr.s_addr = x;		//	INADDR_ANY;         // inet_addr(listen_ip_address == ""? INADDR_ANY : listen_ip_address.c_str());
	empty_addr->sin_port = htons(socket_port_number);
	return true;

}

// Returns socket file descriptor, or -1 on error.
// We use the logger so that we can capture errno as early as possible
// after a system call.
int EnetUtil::server_listen(Log::Logger& logger, struct ::sockaddr *address, int backlog_size)
{
	struct ::sockaddr_in *address_struct = (::sockaddr_in *) address;

	int errnocopy = 0;			// Captures errno right after system call
	int socket_fd = -1; 		// socket file descriptor

	// For setsockopt(2), this parameter should be non-zero to enable a boolean option,
	// or zero if the option is to be disabled
	int optval = 1;

	// signal(SIGPIPE, SIG_IGN);  // this will affect all running threads

    // Creating socket file descriptor
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
    	errnocopy = errno;
    	Util::Utility::get_errno_message(errnocopy);
		logger.error() << "In serverListen(): socket() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &optval, sizeof(optval)))
    {
    	errnocopy = errno;
    	Util::Utility::get_errno_message(errnocopy);
		logger.error() << "In serverListen(): setsockopt() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (bind(socket_fd, (struct sockaddr *)address_struct, sizeof(::sockaddr_in)) < 0)
    {
    	errnocopy = errno;
    	Util::Utility::get_errno_message(errnocopy);
		logger.error() << "In serverListen(): bind() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }

    if (listen(socket_fd, backlog_size) < 0)
    {
    	errnocopy = errno;
    	Util::Utility::get_errno_message(errnocopy);
		logger.error() << "In serverListen(): listen() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
    }
    logger.notice() << "In serverListen(): Server created and accepting connection requests";
    return socket_fd;
}

// Accepts a single connection request.  It is expected to be called
// from within a loop.  For each loop cycle, the caller would
// then start a thread to deal with the request right after this call,
// if and only if the file descriptor is a valid positive file descriptor
// from a successful accept() system call, and the ::sockaddr_in structure
// is valid.
int EnetUtil::server_accept(Log::Logger& logger, int listen_socket_fd, struct ::sockaddr *address)
{
	struct ::sockaddr_in *address_struct = (::sockaddr_in *) address;

	int errnocopy = 0;			// Captures errno right after system call
	int accept_socket_fd = -1;
    int address_length = sizeof(::sockaddr_in);

	if ((accept_socket_fd = accept(listen_socket_fd,
			                       (struct sockaddr *) address_struct,
							       (socklen_t *) &address_length)) < 0)
	{
    	errnocopy = errno;
    	Util::Utility::get_errno_message(errnocopy);
		logger.error() << "In serverAccept(): accept() failed: " << Util::Utility::get_errno_message(errnocopy);
        return -1;
	}

	//////////// std::thread deviceListnerThread(DeviceServer::deviceSocketThreadHandler, accept_socket_fd, shared_from_this());
	//////////// deviceListnerThread.detach();

	return accept_socket_fd;
}

// END OF SOCKET UTILITIES

/////////////////////////////////////////////////////////////////////////////////
// On the assumption that the process self mac address does not change during its
// lifetime...  This static is not visible outside the scope of this source file.
// There usually is more than one mac address.  
static std::vector<std::string> static_self_mac_addresses;

// This utility lock is static, used by get_self_mac_address below
// This static is not visible outside the scope of this source file.
static std::recursive_mutex mac_address_mutex;

// Returns empty vector if not found
std::vector<std::string> EnetUtil::get_all_self_mac_addresses()
{
	using namespace Util;

    if (static_self_mac_addresses.size() != 0)
    {
        // Once set, we don't allow the system's mac addresses to change for the lifetime of the process.
    	std::cerr << "ERROR.  The MAC address object is not empty." << std::endl;
        return static_self_mac_addresses;
    }

    std::lock_guard<std::recursive_mutex> lock(mac_address_mutex);

    // Belt and suspenders (this is about the lock btw --
    // No need to lock/unlock if there were an error. So do it again once locked.
    if (static_self_mac_addresses.size() != 0)
    {
        // Once set, we don't allow the system's mac addresses to change for the lifetime of the process.
    	std::cerr << "ERROR.  The MAC address object is not empty." << std::endl;
        return static_self_mac_addresses;
    }

    const int MIN_MAC_ADDR_LENGTH = 12;
    std::string macAddress;
    std::vector<std::string> macAddressVector;

    std::string command = "ip link show | awk '$0 ~ /ether/ {print $2}'";
    std::string tmpstr;
    char var[16 * 1024];      // This just moves the stack pointer. Size is not an issue here.

    auto fp = popen(command.c_str(), "r");

    if (fp == NULL)
    {
    	std::cerr << "ERROR.  errno: " << Utility::get_errno_message(errno) << std::endl;
        if (macAddressVector.size() == 0)
        {
        	return macAddressVector;   // it's empty
        }
    }
    else
    {
    	while (fgets(var, sizeof (var) - 1, fp) != NULL)
		{
			tmpstr = const_cast<const char *>(var);
			tmpstr = Utility::trim(tmpstr, " \t\n\v\f\r");
			tmpstr = Utility::replace_all(tmpstr, ":", "");  // Remove all : chars - we'll add them back later on....

			if (tmpstr.length() != MIN_MAC_ADDR_LENGTH)
			{
				continue;
			}
			Utility::to_upper(tmpstr);
			macAddress = tmpstr.substr(0, 2) + ":" + tmpstr.substr(2, 2) + ":" + tmpstr.substr(4, 2) + ":"
					+ tmpstr.substr(6, 2) + ":" + tmpstr.substr(8, 2) + ":" + tmpstr.substr(10, 2);
			macAddressVector.push_back(macAddress);
		}
    }

    if (fp != NULL) pclose(fp);
    if (macAddressVector.size() == 0)
    {
    	return macAddressVector;   // it's empty
    }

    static_self_mac_addresses = macAddressVector;
    return static_self_mac_addresses;
}

