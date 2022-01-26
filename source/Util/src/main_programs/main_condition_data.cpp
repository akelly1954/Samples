// "g++ --std=c++1z -pthread program.cpp -lpthread -lstdc++fs -L /path/where/libUtil.so/resides/").

#include "condition_data.hpp"
#include "Utility.hpp"
#include <thread>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

int main(int, char**)
{
	using namespace Util;

    int numthreads = 15;
    std::vector<int> sleeptimes;
    condition_data<int> condvar(0); // loads 0 as the initial data

    for (int i = 0; i<numthreads; i++)
    {
        sleeptimes.push_back(Utility::get_rand(300));
    }

    // vector container stores threads
    std::vector<std::thread> workers;

    for (int i = 1; i <= numthreads; i++) {
        workers.push_back( std::thread([i,sleeptimes,&condvar]()
					{
						int slp = sleeptimes[i-1];
						std::this_thread::sleep_for(std::chrono::milliseconds(slp));

						condvar.wait_for_ready();

						// It's ready - get the data and reset it to a new value
						int oldvalue = condvar.get_data();

						// And set it free
						condvar.send_ready(i);

						//using this so that all thread output comes out on one line
						std::ostringstream lostr;
						lostr << "thread function " << i << "  --  sleeping for " << slp << " -- got " << oldvalue;

						std::cout << lostr.str() << std::endl;
					}));
    }
    std::cout << "main thread\n";

    // Set the condition variable loose....
    condvar.send_ready(0);

    // wait for the last condition to be met...
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    condvar.wait_for_ready();
    std::cout << "Last condition data: " << condvar.get_data() << std::endl;

    std::for_each(workers.begin(), workers.end(), [](std::thread &t)
            {
                t.join();
            });

    return 0;
}
