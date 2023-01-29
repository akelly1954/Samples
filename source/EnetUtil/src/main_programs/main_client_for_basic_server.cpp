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
#include <sys/stat.h>
#include <time.h>

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

std::string default_log_level = "notice";
std::string log_level = default_log_level;
Log::Log::Level loglevel = Log::Log::eNotice;

// fixed size of the std::array<> used for the data
const int server_buffer_size = NtwkUtilBufferSize;

void Usage(std::ostream &strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help (or -h or help)" << std::endl;
    strm << "Or:       " << command
            << "\n"
            << "              -fn file-name             (MANDATORY: both the \"-fn\" flag and the name of the \n"
            << "                                        existing file containing the data to be transmitted \n"
            << "                                        have to be specified on the command line)\n"
            << "              [ -ip server-ip-address ] (default is \"IADDR_ANY\" same as \"\")\n"
            << "              [ -pn port-number ]       (port num to connect to, default is the port number \n"
            << "                                        used by the server - see NOTE below)\n"
            << "              [ -lg log-level ]         (see below, default is \"notice\"\n"
            << "\n"
            << "log-level can be one of: {\"debug\", \"info\", \"notice\", \"warning\", \"error\", \"critical\"}\n"
            << "\n"
            << "NOTE: the default port numbers that both client and server use match up at the time the sources were built.\n"
            << "      If the port number is set on the command line, it should be done for both client and server.\n"
            << "      The server always reports the port number it is listening on in the first few lines of its log file.\n"
            "\n"
            << std::endl;
}

// See the end of this source file for this function
bool parse(std::ostream &strm, Util::CommandLine& cmdline);

bool check_input_file(std::ostream &strm, std::string input_filename, struct stat *sb, size_t & numbytesinfile)
{
    using Util::Utility;

    int errnocopy = 0;

    if (stat(input_filename.c_str(), sb) == -1)
    {
        errnocopy = errno;
        strm << "\nCannot perform stat() on input file \"" << input_filename << "\": "
                << Utility::get_errno_message(errnocopy) << "\n"
                << std::endl;
        return false;
    }

    if (! S_ISREG(sb->st_mode))
    {
        strm << "\nFile " << input_filename << " is not a regular file.\n" << std::endl;
        return false;
    }

    if (sb->st_size >= (off_t) 0x7fffffffffffffff)
    {
        strm << "\nFile " << input_filename << " is too big. Max size allowed is 2*32-1 (32 bits).\n" << std::endl;
        return false;
    }

    numbytesinfile = (size_t) sb->st_size;
    return true;
}

int main(int argc, const char *argv[])
{
    using namespace Util;

    /////////////////
    // Parse command line
    /////////////////

    std::string argv0 = argv[0];

    const std::vector<std::string> allowedFlags ={ "-fn", "-ip", "-pn", "-lg" };
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

    if (!parse(std::cerr, cmdline))
    {
        Usage(std::cerr, argv0);
        std::cerr << "\n" << argv0 << ":  Command line parsing failed." << std::endl;
        return EXIT_FAILURE;
    }

    /////////////////
    // Open input file
    /////////////////

    // This section is here (early) because if there's an error in it, we don't
    // want to set up the logger, or open up a network connection before exiting.
    int errnocopy = 0;
    FILE *input_stream = NULL;

    // to get the file size
    struct stat sb;
    size_t numbytesinfile = 0;

    if (! check_input_file(std::cerr, input_filename, &sb, numbytesinfile))
    {
        // The check_input_file function writes all errors to std::cerr
        Usage(std::cerr, argv0);
        return 1;
    }

    if ((input_stream = ::fopen(input_filename.c_str(), "r")) == NULL)
    {
        errnocopy = errno;
        std::cerr << "\nCannot open input file \"" << input_filename << "\": "
                << Utility::get_errno_message(errnocopy) << "\n"
                << std::endl;
        Usage(std::cerr, argv0);
        return 1;
    }

    /////////////////
    // Set up logger
    /////////////////

    Log::Config::Vector configList;
    MainLogger::initializeLogManager(configList, loglevel, logFileName, MainLogger::enableConsole, MainLogger::disableLogFile);
    MainLogger::configureLogManager(configList, logChannelName);
    Log::Logger logger(logChannelName);

    logger.debug() << "Using " << input_filename << " for input. Size is " << numbytesinfile << " bytes.";

    /////////////////
    // Set up connection to server
    /////////////////

    if (connection_ip.empty() || connection_ip == "INADDR_ANY")
    {
        connection_ip = "";
        logger.debug() << "    Client connecting to ip: INADDR_ANY:" << connection_port_number;
    }
    else
    {
        logger.debug() << "    Client connecting to ip: " << connection_ip << ":" << connection_port_number;
    }

    struct ::sockaddr_in sin_addr;
    if (!NtwkUtil::setup_sockaddr_in(std::string(connection_ip),
            (uint16_t) connection_port_number, (sockaddr*) &sin_addr))
    {
        logger.error()
                << "Error returned from setup_sockaddr_in(): Setup connection for "
                << connection_ip << ":" << connection_port_number
                << " failed. Aborting...";
        if (input_stream != NULL) ::fclose(input_stream);
        return 1;
    }

    int socket_fd = -1;
    if ((socket_fd = NtwkUtil::client_socket_connect(logger, (sockaddr*) &sin_addr)) < 0)
    {
        logger.error()
                << "Error returned from client_socket_connect(): Connection to "
                << connection_ip << ":" << connection_port_number
                << " failed. Aborting...";
        if (input_stream != NULL) ::fclose(input_stream);
        return 1;
    }

    logger.notice() << "Client connected to " << connection_ip << ":"
            << connection_port_number << " Successfully.";


    std::vector<std::string> path = Utility::split(input_filename, "/");
    std::string file_basename = path.back();
    std::string initialMessage = file_basename + "|" + std::to_string(numbytesinfile);

    // Log output has been written already
    if (!NtwkUtil::send_ntwk_message(logger, socket_fd, initialMessage))
    {
        if (input_stream != NULL) ::fclose(input_stream);
        if (socket_fd >= 0) ::close(socket_fd);
        return 1;
    }

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

    logger.notice() << argv0 << ": Successfully sent file \"" << input_filename
            << "\" with " << totalbytes_sent << " bytes to " << connection_ip
            << ":" << connection_port_number;

    // get server response in a string
    std::string response;
    if (NtwkUtil::get_ntwk_message(logger, socket_fd, response))
    {
        logger.notice() << "Server response (for file \"" << input_filename << "\"): " << response;
    }
    else
    {
        logger.error() << response;
    }

    /////////////////
    // Cleanup
    /////////////////

    if (input_stream != NULL)
        ::fclose(input_stream);

    if (socket_fd >= 0)
        ::close(socket_fd);

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return ret;
}

bool parse(std::ostream &strm, Util::CommandLine& cmdline)
{
    using namespace Util;


    int fail_int = 101010;      // this is just for the assert()s

    switch(cmdline.get_template_arg("-ip", connection_ip))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-ip flag not provided. Using default " << connection_ip << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-ip flag provided. Using " << connection_ip << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-ip\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (fail_int == -666);   // will cause abnormal termination
    }

    switch(cmdline.get_template_arg("-pn", connection_port_number))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-pn flag not provided. Using default " << connection_port_number << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-pn flag provided. Using " << connection_port_number << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-pn\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (fail_int == -667);   // Bug encountered. Will cause abnormal termination
    }

    // this flag (-fn) and an existing readable regular file name are MANDATORY
    switch(cmdline.get_template_arg("-fn", input_filename))
    {
        case Util::ParameterStatus::FlagNotProvided:
            strm << "ERROR: the \"-fn\" flag is missing. Specifying input file name with the -fn flag is mandatory." << std::endl;
            return false;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-fn flag provided. Using " << input_filename << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-fn\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (fail_int == -668);   // Bug encountered. Will cause abnormal termination
    }

    switch(cmdline.get_template_arg("-lg", log_level))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-lg flag not provided. Using default " << log_level << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-lg flag provided. Using " << log_level << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-lg\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (fail_int == -669);   // Bug encountered. Will cause abnormal termination
    }

    /////////////////
    // Check out specified log level
    /////////////////

    if (log_level == "debug") loglevel = Log::Log::eDebug;
    else if (log_level == "info") loglevel = Log::Log::eInfo;
    else if (log_level == "notice") loglevel = Log::Log::eNotice;
    else if (log_level == "warning") loglevel = Log::Log::eWarning;
    else if (log_level == "error") loglevel = Log::Log::eError;
    else if (log_level == "critical") loglevel = Log::Log::eCritic;
    else
    {
        strm << "ERROR: Incorrect use of the \"-lg\" flag." << std::endl;
        return false;
    }

    return true;
}

