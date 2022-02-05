
// "g++ --std=c++1z -pthread program.cpp -lpthread -lstdc++fs -L /path/where/libUtil.so/resides/"

#include <thread>
#include <vector>
#include <utility>
#include <algorithm>
#include <mutex>
#include <string>
#include <sstream>
#include <ostream>
#include <iostream>
#include <chrono>
#include <stdlib.h>
#include "condition_data.hpp"
#include "Utility.hpp"
#include "commandline.hpp"

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

// To run this program use:
//
// 		main_condition_data [num_threads]
//
// If num_threads is specified, it has to be numerical and greater than 0. If not, it
// defaults to 20.
//
// For example:
// $
// $ cd /path/to/build/localrun
// $
// $ LD_LIBRARY_PATH=".:" ./main_condition_data 60
//
// By default, the output is sorted chronologically (when each thread finishes its work).
// To sort the output by the thread number, pipe the output:                   | sort -k2 -n
// To sort the output by the milliseconds' delay, pipe the output:             | sort -k6 -n
// To sort the output by the previous thread number from the condition variable, pipe the output:
//                                                                             | sort -k11 -n
//
// NOTE FROM THE AUTHOR:  I got the program to fail on a std::bad_alloc at somewhere between
// 30,000 and 40,000 threads running (succeeded at 30,000, failed on 40,000).  Very specific
// to the system that was used of course (8 core i7 cpu, 32Gb mem, Debian 11 linux).  YMMV.
//

void Usage(std::ostream& strm, std::string command)
{
    strm << "Usage:    " << command << " [ num_threads ]\n\n" <<
    		     "If num_threads is specified, it has to be numerical and greater than 0. ]\n" <<
				 "If it is not specified, it defaults to 20.\n" << std::endl;
}

// Returns the number of threads requested, or 0 if no parameters.
// Returns -1 if Usage() (help) was requested.
int parse(int argc, const char *argv[])
{
    // If no parameters were supplied.
	if (argc == 1)
	{
		return 0;
	}
	else if (std::string(argv[1]) == "--help" ||
  	    	 std::string(argv[1]) == "-h" ||
  	    	 std::string(argv[1]) == "help")
    {
        Usage(std::cerr, argv[0]);
        return -1;
    }
	return strtol(argv[1], NULL, 10);
}


int main(int argc, const char *argv[])
{
	using namespace Util;

    int numthreads = parse(argc, argv);
    // DEBUG   std::cerr << "Parse returned: " << numthreads << std::endl;

    if (numthreads == -1)
    {
    	Usage(std::cerr, argv[0]);
    	return 1;
    }
    else if(numthreads == 0)
    {
    	numthreads = 20;
    }

    std::cerr << "Number of threads chosen: " << numthreads << std::endl;

	// Used to lock the vector of threadData pair<> results below
	std::mutex m_ready_mutex;

	// The unit of data accumulated in each thread
    typedef std::pair<int, std::string> threadData;

    // accumulation of result pairs protexted by m_ready_mutex
    std::vector<threadData> resultPairsVector;

    std::vector<int> sleeptimes;
    condition_data<threadData> condvar(threadData(0, std::string("Initial - this is not from a thread")));

    for (int i = 0; i < numthreads; i++)
    {
        sleeptimes.push_back(Utility::get_rand(300));
    }

    // vector container stores threads
    std::vector<std::thread> workers;

    // First create all the threads, enter them, and then wait
    // for the go signal from the main program.
    for (int i = 1; i <= numthreads; i++) {
    			    workers.push_back( std::thread([i,sleeptimes,&condvar, &m_ready_mutex, &resultPairsVector]()
					{
						int slp = sleeptimes[i-1];

						// The point behind this sleep is for all threads to be started
						// and initialized before continuing
						std::this_thread::sleep_for(std::chrono::milliseconds(slp));

						// After sleeping for various times, all threads wait for the main()
						// function to see if they should start. The condition variable allowing
						// them to continue is set by the main() function once, after all threads have
						// been initialized.
						condvar.wait_for_ready();

						// It's ready - get the data and reset it to a new value
						threadData oldvalue = condvar.get_data();

						//using this so that each thread output comes out on a single line
						std::ostringstream lostr;
						lostr << "thread " << i << " slept " << slp <<
								 " msec -- from thread " <<
								 oldvalue.first << " = \"" << oldvalue.second.substr(0, 74) << "...";
						threadData newvalue = threadData(i, lostr.str());

						{ // scope braces for the lock
							std::lock_guard<std::mutex> lock(m_ready_mutex);
							resultPairsVector.push_back(newvalue);
						}

						// And set it free
						condvar.send_ready(newvalue);
					}));
    }
    std::cerr << "main thread: Sending ready message to all threads\n";

    // All threads have been initialized.  Set the condition variable loose....
    condvar.send_ready(condvar.get_data());

    // wait for the last condition to be met...
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    condvar.wait_for_ready();
    std::cerr << "Last condition thread done" << std::endl;

    for (auto & element: resultPairsVector)
    {
		std::lock_guard<std::mutex> lock(m_ready_mutex);
    	std::cout << "Thread " << element.first << ": \"" << element.second << "\"" << std::endl;
    }

    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
    	if (t.joinable()) t.join();
    });

    return 0;
}
