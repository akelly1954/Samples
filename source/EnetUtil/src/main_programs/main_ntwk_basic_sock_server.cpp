#include <ntwk_basic_sock_server/ntwk_connection_thread.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
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

std::string default_log_level = "notice";
std::string log_level = default_log_level;

const char *default_server_listen_ip = "INADDR_ANY";    // default listen ip address type
std::string server_listen_ip(default_server_listen_ip); // can be modified from the command line

const uint16_t default_server_listen_port_number = simple_server_port_number;
uint16_t server_listen_port_number = default_server_listen_port_number; // can be modified from the command line

const int default_server_listen_max_backlog = 50;       // Maximum number of connection requests queued
int server_listen_max_backlog = 50;                     // can be modified from the command line

// fixed size of the std::vector<> used for the data
const int server_buffer_size = NtwkUtilBufferSize;


void Usage(std::ostream& strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help (or -h or help)" << "\n";
    strm << "Or:       " << command << "\n"
            "                  [ -ip server-ip-address ] (listen ip address (default is \"IADDR_ANY\" same as \"\") \n" <<
            "                  [ -pn port-number ]       (listen port number, default is the port number defined \n" <<
            "                                            during the build - see NOTE below)\n" <<
            "                  [ -bl connections ]       (maximum number of connection requests queued before \n" <<
            "                                            requests are dropped - default is 50) \n" <<
            "                  [ -lg log-level ]         (see below, default is \"notice\"\n" <<
            "\n" <<
            "log-level can be one of: {\"debug\", \"info\", \"notice\", \"warning\", \"error\", \"critical\"}\n" <<
            "\n"
            "NOTE: the default port numbers that both client and server use match up at the time the sources were built.\n" <<
            "      If the port number is set on the command line, it should be done for both client and server.\n" <<
            "      The server always reports the port number it is listening on in the first few lines of its log file.\n" <<
            "\n";
}

bool parse(int argc, const char *argv[], Util::CommandLine& cmdline )
{
    using namespace Util;

    if(cmdline.isError())
    {
        std::cerr << "\n" << argv[0] << ": " << cmdline.getErrorString() << std::endl;
        return false;
    }

    if(cmdline.isHelp())
    {
        // No error, just help
        if (argc > 2)
        {
            std::cerr << "\nWARNING: using the --help flag negates consideration of all other flags and parameters.  Exiting...\n" << std::endl;
        }
        return false;
    }

    std::map<std::string,bool> specified;

    specified["-ip"] = cmdline.get_template_arg("-ip", server_listen_ip);
    specified["-pn"] = cmdline.get_template_arg("-pn", server_listen_port_number);
    specified["-bl"] = cmdline.get_template_arg("-bl", server_listen_max_backlog);
    specified["-lg"] = cmdline.get_template_arg("-lg", log_level);

    bool ret = true;  // Currently all flags have default values, so it's always good.
    std::for_each(specified.begin(), specified.end(), [&ret](auto member) { if (member.second) { ret = true; }});
    return ret;
}

int main(int argc, const char *argv[])
{
    using namespace Util;

    std::string argv0 = argv[0];
    const StringVector allowedFlags ={ "-ip", "-pn", "-bl", "-lg" };
    CommandLine cmdline(argc, argv, allowedFlags);

    if(cmdline.isError())
    {
        std::cout << "\n" << argv0 << ": " << cmdline.getErrorString() << "\n" << std::endl;
        Usage(std::cout, argv0);
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    if(cmdline.isHelp())
    {
        if (argc > 2)
        {
            std::cout << "\nWARNING: using the --help flag cancels all other flags and parameters.  Exiting...\n" << std::endl;
        }
        Usage(std::cout, argv0);
        std::cout << std::endl;
        return EXIT_SUCCESS;
    }

    if (! parse(argc, argv, cmdline))
    {
        std::cout << "\n";
        Usage(std::cout, argv0);
        std::cout << std::endl;
        return 1;
    }

    /////////////////
    // Check out specified log level
    /////////////////

    Log::Log::Level loglevel = Log::Log::eNotice;

    if (log_level == "debug") loglevel = Log::Log::eDebug;
    else if (log_level == "info") loglevel = Log::Log::eInfo;
    else if (log_level == "notice") loglevel = Log::Log::eNotice;
    else if (log_level == "warning") loglevel = Log::Log::eWarning;
    else if (log_level == "error") loglevel = Log::Log::eError;
    else if (log_level == "critical") loglevel = Log::Log::eCritic;
    else
    {
        std::cerr << "\nIncorrect use of the \"-lg\" flag." << std::endl;
        Usage(std::cerr, argv0);
        return 1;
    }

    /////////////////
    // Set up logger
    /////////////////

    Log::Config::Vector configList;

    MainLogger::initializeLogManager(configList, loglevel, logFileName, MainLogger::disableConsole, MainLogger::enableLogFile);
    MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);
    std::cout << "Log level is set to \"" << log_level << "\"" << std::endl;

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

    /////////////////
    // Set up and bind the listener socket on which client connection requests are made
    /////////////////

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

        // Start a thread to handle the connection. Each one of these threads, once
        // they're started (in sequence), gets all the data from the connection and
        // writes it out to a file.
        socket_connection_thread::start (accept_socket_fd, i, logChannelName);
    }

    int ret = (aborted? 1 : 0);

    /////////////////
    // Cleanup
    /////////////////

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();
    socket_connection_thread::terminate_all_threads();

    return ret;
}


#ifdef SAMPLE_RUN

Coming soon...  :-)

#endif //  SAMPLE_RUN



