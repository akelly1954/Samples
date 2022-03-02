
#pragma once

#include <Utility.hpp>
#include <circular_buffer.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <memory>
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

namespace EnetUtil {

	class queue_thread
	{
	public:

		static void handler(Log::Logger logger);

		static void start (const char *logChannelName = "queue_thread");

		static bool write_to_file(  Log::Logger& logger,
									std::shared_ptr<fixed_uint8_array_t> data_sp,
									std::string output_file);

		static std::string get_seq_num_string(long num);	// utility function

		// The circular buffer queue has each shared_ptr<> added to it.  Each shared_ptr
		// points to a fixed_array object which holds data from a single socket read()
		// call, when it's ready.  The queue_handler thread pops out each member when it's
		// ready, and processes it.
		static const int s_queuesize;
		static Util::circular_buffer<std::shared_ptr<fixed_uint8_array_t>> s_ringbuf;

		// Adding a buffer to the queue with this condition variable as a second parameter
		// to ::put(), will allow the waiting queue thread to be suspended until there are
		// buffers waiting in the queue
		static Util::condition_data<int> s_queue_condvar;

		static std::thread s_queue_thread;
	};  // end of class queue_thread

} // end of namespace EnetUtil

