#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

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

//
// For example of use, see the main program in main_programs/main_condition_data.cpp
//
namespace Util {

/////////////////////////////////////////////////////////////////////////////////
// Class condition_data - use a std::condition variable to synchronize
// between events on potentially different threads.  The condition_data class
// also ensures the passing of a data object of type T from the ready-signalling
// caller, to the waiting caller.
//
// Whereas the condition_data class itself is NOT copyable, the encapsulated
// data object (of type T), HAS to be copyable, and copy-constructable.
/////////////////////////////////////////////////////////////////////////////////

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


