#include "Utility.hpp"
#include "commandline.hpp"
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

using namespace EnetUtil;

// Defaults and other constants
														// All threads, including main, output to this channel
const char *logChannelName = "main_basic_socket_server";

const char *logFileName = "main_basic_socket_server_log.txt";

const char *default_server_listen_ip = "127.0.0.1";		// default listen ip address
std::string server_listen_ip(default_server_listen_ip);	// can be modified from the command line

const uint16_t default_server_listen_port_number = EnetUtil::simple_server_port_number;
uint16_t server_listen_port_number = default_server_listen_port_number; // can be modified from the command line

const int default_server_listen_max_backlog = 50;		// Maximum number of connection requests queued
int server_listen_max_backlog = 50;						// can be modified from the command line

// fixed size of the std::vector<> used for the data
const int server_buffer_size = EnetUtil::NtwkUtilBufferSize;

// vector container stores the threads that do the work for each connection
std::vector<std::thread> workers;

void Usage(std::ostream& strm, std::string command)
{
    strm << "Usage:    " << command << " --help (or -h or help)" << std::endl;
    strm << "Or:       " << command << "\n"
    			 "                  -ip server listen ip address (default is 127.0.0.1) \n" <<
				 "                  -pn port number server is listening on\n" <<
				 "                  -bl maximum number of connection requests queued (before \n" <<
				 "                      requests are dropped - default is 50) \n" <<
				 "\n" <<
				 std::endl;
}

bool parse(int argc, char *argv[])
{
    using namespace Util;
    const std::map<std::string,std::string> cmdmap = getCLMap(argc, argv);
    std::map<std::string,bool> specified;

    specified["-ip"] = getArg(cmdmap, "-ip", server_listen_ip);
	specified["-pn"] = getArg(cmdmap, "-pn", server_listen_port_number);
    specified["-bl"] = getArg(cmdmap, "-bl", server_listen_max_backlog);

    bool ret = true;  // Currently all flags have default values, so it's always good.
    std::for_each(specified.begin(), specified.end(), [&ret](auto member) { if (member.second) { ret = true; }});
    return ret;
}

void thread_handler(int socketfd, int threadno)         ////////         , Log::Logger& logger)
{
	std::cout << "thread_handler(): started thread for connection " << threadno << ", fd = " << socketfd << std::endl;
	// logger.notice() << "thread_handler(): start thread for connection " << threadno << ", fd = " << socketfd;
	;
}

void socket_connection_handler (int socket, int threadno)
{
        // This affects the whole process:  signal(SIGPIPE, SIG_IGN);

   	Log::Logger logger(logChannelName);
   	logger.notice() << "socket_connection_handler(): starting a connection handler thread";

	try
	{
    	workers.push_back( std::thread( thread_handler, socket, threadno));             ////////    , logger));
    }
    catch (std::exception &exp)
    {
        logger.error() << "Got exception in socket_connection_handler() starting thread " <<
        				  threadno << " for socket fd " << socket << ": " << exp.what();
    }
    catch (...)
    {
    	logger.error() << "General exception occurred in socket_connection_handler() starting thread " <<
        				  threadno << " for socket fd " << socket;
    }

    logger.notice() << "socket_connection_handler(): started thread " <<
        				  threadno << " for socket fd " << socket;
}


int main(int argc, char *argv[])
{
	using namespace Util;

    std::string argv0 = const_cast<const char *>(argv[0]);

    // If no parameters were supplied, or help was requested:
    if (argc > 1 &&
    		(std::string(const_cast<const char *>(argv[1])) == "--help" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "-h" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "help")
		)
    {
        Usage(std::cerr, argv0);
        return 0;
    }

    bool parseres = parse(argc, argv);
    if (! parseres)
    {
        Usage(std::cerr, argv0);
        return 1;
    }

    Log::Config::Vector configList;
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, logFileName, false, true);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

    logger.notice() << "======================================================================";
    logger.notice() << "Starting the server:";

    if (server_listen_ip.empty() || server_listen_ip == "INADDR_ANY")
    {
    	server_listen_ip = "";
    	logger.notice() << "	listening on ip address: INADDR_ANY";
    }
    else
    {
    	logger.notice() << "	listening on ip address: " << server_listen_ip;
    }

    logger.notice() << "    port number: " << server_listen_port_number;
	logger.notice() << "	max backlog connection requests: " << server_listen_max_backlog;
    logger.notice() << "======================================================================";


    struct ::sockaddr_in sin_addr;
    if(! setup_listen_addr_in(std::string(server_listen_ip), (uint16_t) server_listen_port_number, (sockaddr *) &sin_addr))
	{
		logger.error() << "Error returned from setup_listen_addr_in(): Aborting...";
		return 1;
	}

    int socket_fd = -1;
    if ((socket_fd = server_listen(logger, (sockaddr *) &sin_addr, server_listen_max_backlog)) < 0)
    {
		logger.error() << "Error returned from server_listen(): Aborting...";
		return 1;
    }

    // From this point on we loop on the accept() system call. starting a thread
    // to handle each connection.
    logger.notice() << "In main(): Server created and accepting connection requests";

	int accept_socket_fd = -1;
    bool aborted = false;

    int i = 0;
    for (i = 1; !aborted; i++)
    {
    	if ((accept_socket_fd = server_accept(logger, socket_fd, (sockaddr *) &sin_addr)) < 0)
        {
    		// Aborting
    		aborted = true;
    		continue;
        }

    	logger.notice() << "In main(): Connection " << i << " accepted: fd = " << accept_socket_fd;

    	// Start a thread to handle the connection
    	socket_connection_handler(accept_socket_fd, i);
    }

    int ret = (aborted? 1 : 0);

    /////////////////
    // Cleanup
    /////////////////

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
    	if (t.joinable()) t.join();
    });

    return ret;
}


#ifdef SAMPLE_RUN


#endif //  SAMPLE_RUN



