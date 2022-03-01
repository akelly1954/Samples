// #include <ntwk_basic_sock_server/ntwk_queue_thread.hpp>
// #include <ntwk_basic_sock_server/ntwk_connection_thread.hpp>
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

// All threads, including main, output to this channel
const char *logChannelName = "main_client_for_basic_server";

const char *logFileName = "main_client_for_basic_server_log.txt";

const char *default_connection_ip = "127.0.0.1";		// default address to connect to
std::string connection_ip(default_connection_ip);    // can be modified from the command line

const uint16_t default_connection_port_number = simple_server_port_number;
uint16_t connection_port_number = default_connection_port_number; // can be modified from the command line

// fixed size of the std::array<> used for the data
const int server_buffer_size = NtwkUtilBufferSize;

void Usage(std::ostream& strm, std::string command)
{
    strm << "Usage:    " << command << " --help (or -h or help)" << std::endl;
    strm << "Or:       " << command << "\n"
                 "                  -ip server ip address (default is 127.0.0.1) \n" <<
                 "                  -pn port number to connect to\n" <<
                 "\n" <<
                 std::endl;
}

bool parse(int argc, char *argv[])
{
    using namespace Util;
    const std::map<std::string,std::string> cmdmap = getCLMap(argc, argv);
    std::map<std::string,bool> specified;

    specified["-ip"] = getArg(cmdmap, "-ip", connection_ip);
    specified["-pn"] = getArg(cmdmap, "-pn", connection_port_number);

    bool ret = true;  // Currently all flags have default values, so it's always good.
    std::for_each(specified.begin(), specified.end(), [&ret](auto member) { if (member.second) { ret = true; }});
    return ret;
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
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eDebug, logFileName, true, false);
    // Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, logFileName, false, true);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

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
    if(! NtwkUtil::setup_sockaddr_in(std::string(connection_ip), (uint16_t) connection_port_number, (sockaddr *) &sin_addr))
    {
        logger.error() << "Error returned from setup_sockaddr_in(): Setup connection for " <<
        				  connection_ip << ":" << connection_port_number << " failed. Aborting...";
        return 1;
    }

    int socket_fd = -1;
    if ((socket_fd = NtwkUtil::client_socket_connect(logger, (sockaddr *) &sin_addr)) < 0)
    {
        logger.error() << "Error returned from client_socket_connect(): Connection to " <<
        				  connection_ip << ":" << connection_port_number << " failed. Aborting...";
        return 1;
    }

    logger.notice() << "Client connected to " << connection_ip << ":" << connection_port_number << " Successfully.";

#ifdef NOBUILD
    //////////////////////////////////////////////////////////////////////////////////////
    // MAIN SERVER LOOP:  All connections have been set up.
    // From this point on we loop on the accept() system call. starting a thread
    // to handle each connection.
    //////////////////////////////////////////////////////////////////////////////////////

    int accept_socket_fd = -1;
    bool aborted = false;

    int i = 0;
    for (i = 1; !aborted; i++)
    {
        if ((accept_socket_fd = NtwkUtil::server_accept(logger, socket_fd, (sockaddr *) &sin_addr)) < 0)
        {
            // Aborting
            aborted = true;
            continue;
        }

        logger.debug() << "In main(): Connection " << i << " accepted: fd = " << accept_socket_fd;

        // Start a thread to handle the connection
        socket_connection_thread::start (accept_socket_fd, i, logChannelName);
    }

    int ret = (aborted? 1 : 0);

    /////////////////
    // Cleanup
    /////////////////

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();
    socket_connection_thread::terminate_all_threads();
    if (queue_thread::s_queue_thread.joinable()) queue_thread::s_queue_thread.join();

    return ret;
}
#endif // NOBUILD
	return 0;
}

#ifdef SAMPLE_RUN

Coming soon...  :-)

#endif //  SAMPLE_RUN



