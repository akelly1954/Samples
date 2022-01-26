#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

namespace Util {

//////////////////////////////////////////////////////////////////////////////////////////
// Class condition_data - use a std::condition variable to synchronize
// between events on potentially different threads.  The condition_data class
// also ensures the passing of a data object of type T from the ready-signalling
// caller, to the waiting caller.
//
// Whereas the condition_data class itself is NOT copyable, the encapsulated
// data object (of type T), HAS to be copyable, and copy-constructable.
/////////////////////////

template<class T>
class condition_data {
private:
    // Not allowed:
    condition_data(void) = delete;
    condition_data(const condition_data &) = delete;
    condition_data &operator=(condition_data const &) = delete;
    condition_data(condition_data &&) = delete;
    condition_data &operator=(condition_data &&) = delete;

public:
    // Once the condition has been satisfied, notify one? or all? of
    // the threads waiting for this event.
    enum NotifyEnum {
        One, All
    };

    condition_data(const T& data) :
            m_ready(false), m_data(data) {
    }

    virtual ~condition_data(void) = default;

    // After the m_ready variable is set to true, m_data is also
    // available, and can be gotten using get_data().
    void wait_for_ready(void) {
        std::unique_lock<std::mutex> lock(m_ready_mutex);
        while (!m_ready) {
            m_ready_condition.wait(lock);
        }
        // This assures us that the next wait() actually waits
        // for the data.
        m_ready = false;
    }

    // Make sure T has a valid operator=(const T&) and T(const T&) methods.
    T get_data(void) const {
        std::lock_guard<std::mutex> lock(m_ready_mutex);
        return m_data;
    }

    // The data variable below is not an indication of "ready" - the mere calling
    // of this method signals "ready".
    // The data object is data to be communicated to the caller of wait_for_ready.
    void send_ready(const T& data, condition_data::NotifyEnum w = One) {
        {
            std::lock_guard<std::mutex> lock(m_ready_mutex);
            m_ready = true;
            m_data = data;
        }
        if (w == condition_data::One) {
            m_ready_condition.notify_one();
        } else {
            m_ready_condition.notify_all();
        }
    }

private:
    std::condition_variable m_ready_condition;
    mutable std::mutex m_ready_mutex;
    bool m_ready;
    T m_data;
};

} // namespace Util

/////////////////////////////////////////////////////////////////////////////
// EXAMPLE OF USE:
/////////////////

#ifdef _EXAMPLE_OF_USE_

// Copy this to a new temporary .cpp file, and build it with the proper flags (use
// "g++ --std=c++1z -pthread program.cpp -lpthread -lstdc++fs -L /path/where/libUtil.so/resides/").

#include "condition_data.h"
#include "Utility.h"
#include <thread>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <iostream>
#include <chrono>

int main(int, char**)
{
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

#endif // _EXAMPLE_OF_USE_

