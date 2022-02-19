#pragma once

#include <LoggerCpp/LoggerCpp.h>
#include <NtwkConnect.hpp>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <stdint.h>

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

	// Options for NtwkUtilBufferSize
	static const size_t NtwkUtilNanoBufferSize = 64;
	static const size_t NtwkUtilMicroBufferSize = 128;
	static const size_t NtwkUtilTinyBufferSize = 256;
	static const size_t NtwkUtilSmallBufferSize = 1024;
	static const size_t NtwkUtilRegularBufferSize = 4096;
	static const size_t NtwkUtilLargeBufferSize = 8192;

	// NOTE: The fixed std::aray<> size is set in the first object defined in class NtwkUtil.
	// 		 It can be changed right there - it will apply to all objects in NtwkUtil.*
	class NtwkUtil
	{
	public:
		// NtwkUtilBufferSize - The fixed size of the std::array<> - baked into the program -
		// can't change (like a #define'd constant...)
		static const size_t NtwkUtilBufferSize = EnetUtil::NtwkUtilRegularBufferSize;

		static int enetSend(Log::Logger& logger,
								int fd,
								std::array<uint8_t,NtwkUtil::NtwkUtilBufferSize>& array_element_buffer,
								std::recursive_mutex& mutex = NtwkUtil::m_recursive_mutex,
								int flag = MSG_NOSIGNAL);

		static int enetReceive(Log::Logger& logger,
								int fd,
								std::array<uint8_t,NtwkUtil::NtwkUtilBufferSize>& array_element_buffer,	// data and length
								size_t requestsize);

		static std::recursive_mutex m_recursive_mutex;
		static std::mutex m_mutex;
	};
} // namespace EnetUtil

