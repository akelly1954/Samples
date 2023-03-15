
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
#include <sys/types.h>
#include <generic_data.hpp>

// This becomes the basis for all other data items
static Util::data_item_container<uint8_t> reference_item;

const char *logChannelName = "util_combo_objects";

void printHeader(const std::string& label)
{
    std::cout << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;
    std::cout << "|   " << label << std::endl;
    std::cout << "------------------------------------------------------------------" << std::endl;
}

void printdata(const std::string& label, Util::data_item_container<u_int8_t> sd)
{
    std::cout << "\nContents of " << label << ": " << std::endl;
    if (! sd.is_valid())
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


void initializeDataItem(Util::data_item_container<uint8_t>& ditem, uint8_t ival = 0 )
{
    using namespace Util;

    u_int8_t uint8ctr = ival;
    for (auto itr = ditem._begin(); itr != ditem._end(); itr++)
    {
        if ((uint8ctr % 50) == 0) uint8ctr = 0;
        *itr = uint8ctr;
        uint8ctr++;
    }
}

// the parameter somedata becomes a copy of the original (not reference)
void checkDataItem(Util::data_item_container<uint8_t> somedata)
{
    using namespace Util;

    printdata("initial somedata", somedata);
    std::cout << "\n====================================" << std::endl;

    data_item_container<u_int8_t>moved_data = std::move(somedata);
    printdata("moved_data", moved_data);

    std::cout << "\n====================================" << std::endl;

    data_item_container<u_int8_t>copied_data(moved_data);
    printdata("copied_data", copied_data);

    std::cout << "\n====================================" << std::endl;

    printdata("invalid_somedata", somedata);

    std::cout << "\n====================================" << std::endl;

}

void checkDataItem(Util::shared_ptr_uint8_data_t some_shared_data)
{
    using namespace Util;

    std::cout << "shared_ptr use count = " << some_shared_data.use_count() << std::endl;
    std::cout << "shared data elements = " << some_shared_data->num_items() << std::endl;
    std::cout << "shared data object is valid: " << Utility::stringify_bool(some_shared_data->is_valid()) << std::endl;

    checkDataItem(*some_shared_data->get_data_item_container());

    // THIS PRODUCES A COMPILE ERROR (use of deleted function) - which is deliberate.
    // shared_ptr_uint8_data_t illegal_sp;
    // *illegal_sp = std::move(*some_shared_data);
}

void Usage(std::ostream& strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help\n" << std::endl;
    strm << "Or: " << std::endl;
    strm << "\n          " << command << " num_data_items\n" << std::endl;
}

bool parse(int argc, const char *argv[], int& num_data_items)
{
    // If no parameters were supplied.
    if (argc == 1)
    {
        std::cerr << "\nError: missing num_data_items parameter.\n" << std::endl;
        Usage(std::cerr, argv[0]);
        return false;
    }
    else if (std::string(argv[1]) == "--help" ||
               std::string(argv[1]) == "-h" ||
               std::string(argv[1]) == "help")
    {
        Usage(std::cerr, argv[0]);
        return false;
    }
    num_data_items = strtol(argv[1], NULL, 10);
    return true;
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

int main(int argc, const char *argv[])
{
    using namespace Util;

    int num_data_items = 0;

    if (!parse(argc, argv, num_data_items))
    {
        return EXIT_FAILURE;
    }
    // DEBUG
    std::cerr << "Parsed command line: num_data_elements = " << num_data_items << std::endl;

    std::string logfilename(logChannelName);
    logfilename += "_log.txt";

    Log::Config::Vector configList;
    Util::MainLogger::initializeLogManager(configList, Log::Log::Level::eDebug, logfilename,
                                           MainLogger::disableConsole, MainLogger::enableLogFile);
    MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName);

    logger.debug() << "Started main_util_combo_objects...";

    // Static global data on the heap.
    // This is the single condition_data object ruling the various running threads
    Util::condition_data<threadData> condvar(threadData(-1, std::string("Initial - this is not from a thread")));

    // random sleep milliseconds for each thread
    std::vector<int> sleeptimes;

    // vector container stores threads
    std::vector<std::thread> workers;

    if (num_data_items < 0)
    {
        std::cerr << "\nError: num_data_items has to be 0 or a positive number.\n" << std::endl;
        Usage(std::cerr, argv[0]);
        return 1;
    }

    std::cerr << "Number of data items chosen: " << num_data_items << std::endl;

//////////////////////////////////////////////////////////

    reference_item = *(new data_item_container<uint8_t>(num_data_items));
    initializeDataItem(reference_item, 0);

    printHeader("Checking reference item");
    checkDataItem(reference_item);

    // shared_data_items<uint8_t> sp_reference = shared_data_items<uint8_t>::create(num_data_items);
    shared_ptr_uint8_data_t sp_reference = shared_uint8_data_t::create(num_data_items);
    shared_ptr_uint8_data_t sp_reference2 = sp_reference->get_shared_ptr();
    shared_ptr_uint8_data_t sp_reference3 = sp_reference->get_shared_ptr();

    printHeader("Checking create(size_t) - shared_ptr to Util::shared_data_items<uint8_t> sp_reference");
    checkDataItem(sp_reference);
    checkDataItem(sp_reference2);
    checkDataItem(sp_reference3);

    printHeader("Checking create(copy object) - shared_ptr to Util::shared_data_items<uint8_t> sp_reference");
    shared_ptr_uint8_data_t sp_alt_reference = shared_uint8_data_t::create(*sp_reference3);
    checkDataItem(sp_alt_reference);

    printHeader("Checking create(iterator range) - shared_ptr to Util::shared_data_items<uint8_t> sp_reference");
    shared_ptr_uint8_data_t sp_alt_reference2 = shared_uint8_data_t::create(sp_alt_reference->_begin(), sp_alt_reference->num_items());
    checkDataItem(sp_alt_reference2);


/////////////////////////////////////////////////////////
#if 0
                  THIS IS WORK IN PROGRESS - NOT DONE YET
#endif
/////////////////////////////////////////////////////////

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return EXIT_SUCCESS;
}

#if 0
                    THIS IS WORK IN PROGRESS - NOT DONE YET

    initializeSleeptimes(numthreads, sleeptimes);

    // First create all the threads, enter them, and then wait
    // for the go signal from the main program. The runThread function
    // does all the work for each thread.
    for (int i = 1; i <= numthreads; i++)
    {
        // Create a Logger object, using a "main_util_combo_objects_log" Channel
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
#endif // 0


