
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
#include <condition_data.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <LoggerCpp/LoggerCpp.h>

// This program runs N (default is 20) concurrent threads, where N is the single command
// line parameter.  All threads are synchronized by a single condition_variable, which
// is also used to communicate a data object (std::pair<int,string>) to the next
// thread which will be allowed to run. Each thread logs the data it receives from
// within the thread.  (See also NOTE FROM THE AUTHOR below).
//
// Before each thread is started, a logger object is instantiated for it, using the
// main logger object instantiated in main().  The end result is that what is logged
// from within the thread is appended to the one log file created by the run.
//
// Console output is disabled - if 10,000 threads are each appending output to
// the output, it messes up the output stream and loses data (which makes sense).
// This would also slow things so much as to invalidate the purpose of the program.
//
// Also disabled is the rotation of log files.  Each run recreates the single log
// file.
//
// To run this program use:
//
//         main_condition_data [num_threads]
//
// The main output is placed in the file "main_condition_data_log.txt" (and a few lines
// containing information are displayed to stdout on the console as the program runs).
//
// If num_threads is specified, it has to be numerical and greater than 0. If not, it
// defaults to 20.
//
// FOR A SAMPLE RUN SHOWING THE RESULTS SEE THE #ifdef'ed SECTION AT THE VERY END OF THIS FILE
//
// NOTE FROM THE AUTHOR:  I got the program to fail on a std::bad_alloc at somewhere between
// 30,000 and 40,000 threads running at the same time (succeeded at 30,000, failed on 40,000).
// There was NO data loss at all at 30,000 (no broken lines in the log file, all content
// in each line was present).
//
// This result is very specific to the system that was used of course (8 core i7 cpu, 32Gb mem,
// Debian 11 linux).
//
// YMMV.
//

///////////////////////////////////////////////////////////////////
// Start of hijacked section (to test data_item_container)
// This might become a separate main program - just not this minute...
// (March 2023)

#include <sys/types.h>
#include <generic_data.hpp>

void printdata(const std::string& label, Util::data_item_container<u_int8_t> sd)
{
    std::cout << "Contents of " << label << ": " << std::endl;
    if (! sd.isValid())
    {
        std::cout << "Parameter " << label << " is invalid (empty)." << std::endl;
        return;
    }

    size_t member = 0;
    for (auto itr = sd._begin(); itr != sd._end(); itr++)
    {
        std::cout << "[" << member << "]=" << std::to_string(*itr) << "  ";
        member++;
    }
    std::cout << std::endl;
}


void initializeGenericData()
{
    using namespace Util;

    data_item_container<u_int8_t> somedata(2*1024);

    u_int8_t uint8ctr = 0;
    for (auto itr = somedata._begin(); itr != somedata._end(); itr++)
    {
        if ((uint8ctr % 0xff) == 0) uint8ctr = 0;
        *itr = uint8ctr;
        uint8ctr++;
    }

    printdata("initial somedata", somedata);

    data_item_container<u_int8_t>moved_data = std::move(somedata);
    printdata("moved_data", moved_data);

    std::cout << "\n\n====================================\n\n" << std::endl;

    data_item_container<u_int8_t>copied_data(moved_data);
    printdata("copied_data", copied_data);

    std::cout << "\n\n====================================\n\n" << std::endl;

    printdata("invalid_somedata", somedata);

    std::cout << "\n\n====================================\n\n" << std::endl;

}
// End of hijacked section (to test data_item_container)
///////////////////////////////////////////////////////////////////

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

// The unit of data passed around for each thread
// using the condition_variable.
typedef std::pair<int, std::string> threadData;

void initializeSleeptimes(int numthreads, std::vector<int>& sleeptimes)
{
    for (int i = 0; i < numthreads; i++)
    {
        sleeptimes.push_back(Util::Utility::get_rand(300));
    }
}

// All threads, including main, output to this channel
const char *logChannelName = "main_condition_data";

int main(int argc, const char *argv[])
{
    using namespace Util;

    int numthreads = parse(argc, argv);
    // DEBUG   std::cerr << "Parse returned: " << numthreads << std::endl;

    Log::Config::Vector configList;
    Util::MainLogger::initializeLogManager(configList, Log::Log::Level::eNotice, "main_condition_data_log.txt", MainLogger::disableConsole, MainLogger::enableLogFile);
    MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

    // Static global data on the heap.
    // This is the single condition_data object ruling the various running threads
    Util::condition_data<threadData> condvar(threadData(-1, std::string("Initial - this is not from a thread")));

    // random sleep milliseconds for each thread
    std::vector<int> sleeptimes;

    // vector container stores threads
    std::vector<std::thread> workers;

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

    // Hijacked this program to also check out the generic data object.
    // uncomment the next line if you want to see lots of output...
    //
    // initializeGenericData();
    //

    initializeSleeptimes(numthreads, sleeptimes);

    // First create all the threads, enter them, and then wait
    // for the go signal from the main program. The runThread function
    // does all the work for each thread.
    for (int i = 1; i <= numthreads; i++)
    {
        // Create a Logger object, using a "main_condition_data_log" Channel
        // This is still running in the main thread - just before the new
        // thread is created
        Log::Logger logger(logChannelName);

        workers.push_back( std::thread([i, &logger, &sleeptimes, &condvar]()
        {
            int threadno = i-1;
            int slp = sleeptimes[threadno];

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

            std::ostringstream lostr;
            lostr << "thread " << threadno << " slept " << slp <<
                     " msec -- from thread " <<
                     oldvalue.first << " = \"" << oldvalue.second.substr(0, 74) << "...";

            std::string result = lostr.str();
            threadData newvalue = threadData(threadno, result);

            // Log it
            logger.notice() << result;

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

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
    {
        if (t.joinable()) t.join();
    });

    return 0;
}

#ifdef SAMPLE_RUN
/*
#
# By default, the output is sorted chronologically (when each thread finishes its work).
#
# To sort the output in main_condition_data_log.txt by the thread number:
#         sort -k6 -n main_condition_data_log.txt
#
# To sort the output in main_condition_data_log.txt by the milliseconds' delay:
#         sort -k8 -n main_condition_data_log.txt
#
# To sort the output in main_condition_data_log.txt by the previous thread number from the condition variable:
#         sort -k13 -n main_condition_data_log.txt
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



