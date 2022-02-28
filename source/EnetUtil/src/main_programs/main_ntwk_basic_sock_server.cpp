#include <ntwk_basic_sock_server/ntwk_queue_thread.hpp>
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
                                                        // All threads, including main, output to this channel
const char *logChannelName = "main_basic_socket_server";

const char *logFileName = "main_basic_socket_server_log.txt";

const char *default_server_listen_ip = "INADDR_ANY";    // default listen ip address type
std::string server_listen_ip(default_server_listen_ip);    // can be modified from the command line

const uint16_t default_server_listen_port_number = simple_server_port_number;
uint16_t server_listen_port_number = default_server_listen_port_number; // can be modified from the command line

const int default_server_listen_max_backlog = 50;        // Maximum number of connection requests queued
int server_listen_max_backlog = 50;                        // can be modified from the command line

// fixed size of the std::vector<> used for the data
const int server_buffer_size = NtwkUtilBufferSize;


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
    Util::Utility::initializeLogManager(configList, Log::Log::Level::eDebug, logFileName, false, true);
    // Util::Utility::initializeLogManager(configList, Log::Log::Level::eNotice, logFileName, false, true);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

    logger.notice() << "======================================================================";
    logger.notice() << "Starting the server:";

    if (server_listen_ip.empty() || server_listen_ip == "INADDR_ANY")
    {
        server_listen_ip = "";
        logger.notice() << "    listening on ip address: INADDR_ANY";
    }
    else
    {
        logger.notice() << "    listening on ip address: " << server_listen_ip;
    }

    logger.notice() << "    port number: " << server_listen_port_number;
    logger.notice() << "    max backlog connection requests: " << server_listen_max_backlog;
    logger.notice() << "======================================================================";

    // Set up the thread that keeps the circular_buffer which handles all the data
    // after it's been received from any accepted socket.
    queue_thread::start (logChannelName);

    struct ::sockaddr_in sin_addr;
    if(! NtwkUtil::setup_sockaddr_in(std::string(server_listen_ip), (uint16_t) server_listen_port_number, (sockaddr *) &sin_addr))
    {
        logger.error() << "Error returned from setup_sockaddr_in(): Aborting...";
        return 1;
    }

    int socket_fd = -1;
    if ((socket_fd = NtwkUtil::server_listen(logger, (sockaddr *) &sin_addr, server_listen_max_backlog)) < 0)
    {
        logger.error() << "Error returned from server_listen(): Aborting...";
        return 1;
    }

    logger.notice() << "In main(): Server created and accepting connection requests";


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


#ifdef SAMPLE_RUN

Coming soon...  :-)

#endif //  SAMPLE_RUN



