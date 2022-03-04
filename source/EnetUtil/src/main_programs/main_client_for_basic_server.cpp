#include <ntwk_basic_sock_server/ntwk_connection_thread.hpp>
#include <Utility.hpp>
#include <commandline.hpp>
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
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <thread>

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

// Defaults and other constants

const char *logChannelName = "main_client_for_basic_server";
const char *logFileName = "main_client_for_basic_server_log.txt";

const char *default_connection_ip = "127.0.0.1"; // default address to connect to
std::string connection_ip(default_connection_ip); // can be modified from the command line

const uint16_t default_connection_port_number = simple_server_port_number;
uint16_t connection_port_number = default_connection_port_number; // can be modified from the command line

std::string input_filename = ""; // This has to be specified on the command line

// fixed size of the std::array<> used for the data
const int server_buffer_size = NtwkUtilBufferSize;

void Usage(std::ostream &strm, std::string command)
{
	strm << "Usage:    " << command << " --help (or -h or help)" << std::endl;
	strm << "Or:       " << command
			<< "\n"
					"                  [ -ip server-ip-address ] (default is 127.0.0.1)\n"
			<< "                  [ -pn port-number ]       (port num to connect to, default is "
			<< connection_port_number << ")\n"
			<< "                  [ -fn file-name           (holding the data to be transmitted, has to exist - default is \"stdin\")\n"
			<< std::endl;
}

bool parse(int argc, char *argv[])
{
	using namespace Util;
	const std::map<std::string, std::string> cmdmap = getCLMap(argc, argv);
	std::map<std::string, bool> specified;

	specified["-ip"] = getArg(cmdmap, "-ip", connection_ip);
	specified["-pn"] = getArg(cmdmap, "-pn", connection_port_number);
	specified["-fn"] = getArg(cmdmap, "-fn", input_filename);

	bool ret = true; // Currently all flags have default values, so it's always good.
	std::for_each(specified.begin(), specified.end(), [&ret](auto member)
	{	if (member.second)
		{	ret = true;}});
	return ret;
}

int main(int argc, char *argv[])
{
	using namespace Util;

	/////////////////
	// Parse command line
	/////////////////

	std::string argv0 = const_cast<const char*>(argv[0]);

	// If no parameters were supplied, or help was requested:
	if (argc > 1
			&& (std::string(const_cast<const char*>(argv[1])) == "--help"
					|| std::string(const_cast<const char*>(argv[1])) == "-h"
					|| std::string(const_cast<const char*>(argv[1])) == "help"))
	{
		Usage(std::cerr, argv0);
		return 0;
	}

	bool parseres = parse(argc, argv);
	if (!parseres)
	{
		Usage(std::cerr, argv0);
		return 1;
	}

	/////////////////
	// Open input file
	/////////////////

	// This section is here (early) because if there's an error in it, we don't
	// want to set up the logger, or open up a network connection before exiting.
	int errnocopy = 0;
	FILE *input_stream = NULL;
	bool fclose_after = false; // This prevents fclose(stdin) from being called.
	if (input_filename.empty())
	{
		fclose_after = false;
		input_stream = ::stdin;
		std::cout << "Using standard input..." << std::endl;
	}
	else if ((input_stream = ::fopen(input_filename.c_str(), "r")) == NULL)
	{
		errnocopy = errno;
		std::cerr << "\nCannot open input file \"" << input_filename << "\": "
				<< Util::Utility::get_errno_message(errnocopy) << "\n"
				<< std::endl;
		Usage(std::cerr, argv0);
		return 1;
	}
	else
	{
		// When input_stream is NOT stdin, this allows fclose(input_stream) to be called
		fclose_after = true;
		std::cout << "Using " << input_filename << " for input..." << std::endl;
	}

	/////////////////
	// Set up logger
	/////////////////

	Log::Config::Vector configList;
	Util::Utility::initializeLogManager(configList, Log::Log::Level::eDebug,
			logFileName, true, false);
	// Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, logFileName, false, true);
	Util::Utility::configureLogManager(configList, logChannelName);
	Log::Logger logger(logChannelName);

	/////////////////
	// Set up connection to server
	/////////////////

	if (connection_ip.empty() || connection_ip == "INADDR_ANY")
	{
		connection_ip = "";
		// logger.debug() << "    Client connecting to ip: INADDR_ANY:" << connection_port_number;
	}
	else
	{
		// logger.debug() << "    Client connecting to ip: " << connection_ip << ":" << connection_port_number;
	}

	struct ::sockaddr_in sin_addr;
	if (!NtwkUtil::setup_sockaddr_in(std::string(connection_ip),
			(uint16_t) connection_port_number, (sockaddr*) &sin_addr))
	{
		logger.error()
				<< "Error returned from setup_sockaddr_in(): Setup connection for "
				<< connection_ip << ":" << connection_port_number
				<< " failed. Aborting...";
		if (fclose_after && input_stream != NULL)
			::fclose(input_stream);
		return 1;
	}

	int socket_fd = -1;
	if ((socket_fd = NtwkUtil::client_socket_connect(logger,
			(sockaddr*) &sin_addr)) < 0)
	{
		logger.error()
				<< "Error returned from client_socket_connect(): Connection to "
				<< connection_ip << ":" << connection_port_number
				<< " failed. Aborting...";
		if (fclose_after && input_stream != NULL)
			::fclose(input_stream);
		return 1;
	}

	logger.notice() << "Client connected to " << connection_ip << ":"
			<< connection_port_number << " Successfully.";

	/////////////////
	// And work: loop data from file to network
	/////////////////

	arrayUint8 array_element_buffer;
	size_t totalbytes_sent = 0;
	int ret = 0;
	while (!std::feof(input_stream))
	{
		// fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
		size_t numread = 0;
		numread = std::fread(array_element_buffer.data(), sizeof(uint8_t),
				array_element_buffer.size(), input_stream);

		if (::ferror(input_stream))
		{
			logger.error() << argv0 << ": Error in reading " << input_filename;
			ret = 1;
			break;
		}

		if (numread > 0)
		{
			ret = NtwkUtil::enet_send(logger, socket_fd, array_element_buffer,
					numread, MSG_NOSIGNAL);
			if (ret < 0)
				break;  // error message logged from inside enet_send()
			else if (ret == 0)
			{
				logger.debug() << argv0 << ": No data was sent to "
						<< connection_ip << ":" << connection_port_number
						<< ", size requested: " << array_element_buffer.size();
				ret = 1;
				break;        // make sure this goes through cleanup below
			}
			else
			{
				// logger.debug() << argv0 << ": Successfully sent " << ret << " bytes to " << connection_ip << ":" << connection_port_number;
			}
			totalbytes_sent += ret;
		}
	}

	logger.debug() << argv0 << ": Successfully sent file \"" << input_filename
			<< "\" with " << totalbytes_sent << " bytes to " << connection_ip
			<< ":" << connection_port_number;

	/////////////////
	// Cleanup
	/////////////////

	if (fclose_after && input_stream != NULL)
		::fclose(input_stream);
	if (socket_fd >= 0)
		::close(socket_fd);

	// Terminate the Log Manager (destroy the Output objects)
	Log::Manager::terminate();

	return ret;
}

#ifdef SAMPLE_RUN

Coming soon...  :-)

#endif //  SAMPLE_RUN

