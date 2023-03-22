#pragma once

/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2022 - 2023 Andrew Kelly
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

#include <Utility.hpp>
#include <condition_data.hpp>
#include <shared_data_items.hpp>
#include <vidcap_profiler_thread.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>
#include <mutex>
#include <vector>
#include <algorithm>

namespace VideoCapture {

    // Queue handler thread
    void raw_buffer_queue_handler();

    // This object serves as the consumer for frame buffers (shared_ptr<>'s)
    // received from the raw queue, and dealt with in this new thread.
    class frame_worker_thread_base
    {
    public:
        frame_worker_thread_base(std::string label, size_t elements_in_ring_buffer = 100)
            : m_condvar(0)
            , m_ringbuf(elements_in_ring_buffer)
            , m_label(label)
            , m_terminated(false)
            , splogger(Util::UtilLogger::getLoggerPtr())
            , initialized(false)
        { ; }
        virtual ~frame_worker_thread_base() = default;
        virtual void setup() = 0;
        virtual void run() = 0;
        virtual void finish() = 0;
        virtual void set_terminated(bool t) = 0;
        virtual void add_buffer_to_queue(Util::shared_ptr_uint8_data_t) = 0;

    public:
        Util::condition_data<int> m_condvar;
        Util::circular_buffer<Util::shared_ptr_uint8_data_t> m_ringbuf;
        std::string m_label;
        bool m_terminated;
        std::mutex worker_base_queue_mutex;
        std::shared_ptr<Log::Logger> splogger;
        bool initialized;
    };

    // The main video frame queueing object.
    class video_capture_queue
    {
    public:

        virtual ~video_capture_queue();

        static void set_terminated(bool t);         // main() sets this to true or false

        static void add_buffer_to_raw_queue(void *p, size_t bsize);

        static void register_worker_thread(std::thread *workerthread);
        static void register_worker(frame_worker_thread_base *worker);

        static std::mutex capture_queue_mutex;
        static bool s_terminated;
        static Util::condition_data<int> s_condvar;
        static Util::circular_buffer<Util::shared_ptr_uint8_data_t> s_ringbuf;

        // pointers to all std::threads started by the raw queue object (this->)
        static std::vector<std::thread *> s_workerthreads;

        // pointers to all frame worker objects started by the raw queue object (this->)
        static std::vector<frame_worker_thread_base *> s_workers;

    };  // end of class video_capture_queue

} // end of namespace VideoCapture







