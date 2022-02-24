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

const int default_server_listen_port_number = EnetUtil::simple_server_port_number;
int server_listen_port_number = default_server_listen_port_number; // can be modified from the command line

const int default_server_listen_max_backlog = 50;		// Maximum number of connection requests queued
int server_listen_max_backlog = 50;						// can be modified from the command line

// fixed size of the std::vector<> used for the data
const int server_buffer_size = EnetUtil::NtwkUtilBufferSize;


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

#ifdef FOR_DEBUG
	for (auto it = specified.begin(); it != specified.end(); ++it)
	{
		std::cout << it->first.c_str() << " = " << (it->second? "true" : "false") << std::endl;
	}
#endif // FOR_DEBUG

    // If any of the parameters were specified, it's ok
    bool ret = false;
    std::for_each(specified.begin(), specified.end(), [&ret](auto member) { if (member.second) { ret = true; }});
    return ret;
}

#ifdef FOR_DEBUG
// random sleep milliseconds for each thread
std::vector<int> sleeptimes;

void initializeSleeptimes(int numthreads, std::vector<int>& sleeptimes)
{
	for (int i = 0; i < numthreads; i++)
	{
		sleeptimes.push_back(Util::Utility::get_rand(300));
	}
}
#endif // FOR_DEBUG


int main(int argc, char *argv[])
{
	using namespace Util;

    std::string argv0 = const_cast<const char *>(argv[0]);

    // If no parameters were supplied, or help was requested:
    if (argc <= 1 || (argc > 1 &&
    		(std::string(const_cast<const char *>(argv[1])) == "--help" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "-h" ||
  	    	 std::string(const_cast<const char *>(argv[1])) == "help")
		)
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
    logger.notice() << "Attempting start of server:";

    if (server_listen_ip.empty())
    {
    	logger.notice() << "	server listening on ip address: IADDR_ANY";
    }
    else
    {
    	logger.notice() << "	server listening on ip address: " << server_listen_ip;
    }

    logger.notice() << "    port number: " << server_listen_port_number;
	logger.notice() << "	max backlog connection requests: " << server_listen_max_backlog;
    logger.notice() << "======================================================================";



    return 0;
}


#ifdef SAMPLE_RUN
/*
#
# By default, the output is sorted chronologically (when each thread finishes its work).
#
# To sort the output in main_condition_data_log.txt by the thread number:
# 		sort -k6 -n main_condition_data_log.txt
#
# To sort the output in main_condition_data_log.txt by the milliseconds' delay:
# 		sort -k8 -n main_condition_data_log.txt
#
# To sort the output in main_condition_data_log.txt by the previous thread number from the condition variable:
# 		sort -k13 -n main_condition_data_log.txt
#
# SAMPLE RUN:
$
$ cd /path/to/build/localrun
$
$ LD_LIBRARY_PATH=".:" ./main_condition_data 10
Number of threads chosen: 10
main thread: Sending ready message to all threads
Last condition thread done


$ cat main_condition_data_log.txt
2022-02-13 07:51:03.982  main_condition_data_log NOTE thread 1 slept 56 msec -- from thread 0 = "Initial - this is not from a thread...
2022-02-13 07:51:03.996  main_condition_data_log NOTE thread 9 slept 70 msec -- from thread 1 = "thread 1 slept 56 msec -- from thread 0 = "Initial - this is not from a th...
2022-02-13 07:51:03.997  main_condition_data_log NOTE thread 4 slept 71 msec -- from thread 9 = "thread 9 slept 70 msec -- from thread 1 = "thread 1 slept 56 msec -- from ...
2022-02-13 07:51:04.030  main_condition_data_log NOTE thread 6 slept 104 msec -- from thread 4 = "thread 4 slept 71 msec -- from thread 9 = "thread 9 slept 70 msec -- from ...
2022-02-13 07:51:04.141  main_condition_data_log NOTE thread 2 slept 216 msec -- from thread 6 = "thread 6 slept 104 msec -- from thread 4 = "thread 4 slept 71 msec -- from...
2022-02-13 07:51:04.160  main_condition_data_log NOTE thread 3 slept 235 msec -- from thread 2 = "thread 2 slept 216 msec -- from thread 6 = "thread 6 slept 104 msec -- fro...
2022-02-13 07:51:04.168  main_condition_data_log NOTE thread 7 slept 242 msec -- from thread 3 = "thread 3 slept 235 msec -- from thread 2 = "thread 2 slept 216 msec -- fro...
2022-02-13 07:51:04.173  main_condition_data_log NOTE thread 0 slept 248 msec -- from thread 7 = "thread 7 slept 242 msec -- from thread 3 = "thread 3 slept 235 msec -- fro...
2022-02-13 07:51:04.194  main_condition_data_log NOTE thread 5 slept 268 msec -- from thread 0 = "thread 0 slept 248 msec -- from thread 7 = "thread 7 slept 242 msec -- fro...
2022-02-13 07:51:04.200  main_condition_data_log NOTE thread 8 slept 274 msec -- from thread 5 = "thread 5 slept 268 msec -- from thread 0 = "thread 0 slept 248 msec -- fro...

$
*/

#endif //  SAMPLE_RUN



