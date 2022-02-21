#pragma once

#include <LoggerCpp/LoggerCpp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <utility>
#include <stdint.h>
#include <assert.h>

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

	// NtwkUtilBufferSize - is the fixed size of the std::array<> - baked into the program -
	// can't change (like a #define'd constant can't change at runtime...). The std::array<>
	// object HAS to be built with a fixed size BY DEFINITION.  We can pretend it's not (like
	// we do here) but it is what it is.  At the very least, if the size is not presented as a
	// #define'd constant, then the variable used for the size has to be known to the
	// compiler at compile time (and should therefore be a const which is assigned a value at
	// compile time).
	//
	// NOTE: The fixed std::aray<> size is set here, and can/should be
	// 		 changed right there - it will apply to all objects in NtwkUtil.*
	static const size_t NtwkUtilBufferSize = EnetUtil::NtwkUtilNanoBufferSize;
	//////   OR - whatever - static const size_t NtwkUtilBufferSize = EnetUtil::NtwkUtilRegularBufferSize;

	// Options for port numbers for listening and connecting to:
	// Do not use directly.  See NtwkUtilBufferSize below.
	static const int base_simple_server_port_number =  5831442;

	// This is the real port number. Both connections and listening use this variable.
	// Make sure to rebuild all clients when connecting to a server using this if it changes.
	static const int simple_server_port_number =
			EnetUtil::base_simple_server_port_number+EnetUtil::NtwkUtilBufferSize;

	// This is the actual type of the data being handled
	typedef std::array<uint8_t,EnetUtil::NtwkUtilBufferSize> arrayUint8;

	///////////////////////////////////////////////////////////////////////////////
	// The object below holds an std::pair<> which manages the std::array<>
	// as well as a count of the number of valid bytes in the array.
	// In this case it's necessary, since (*pair<>.first) will aways have
	// std::array<>::size() elements' capacity in it, but we still need to know
	// and be able to get and set how many members of type T were actually assigned
	// to it, or read into it.
	///////////////////////////////////////////////////////////////////////////////

	template<typename T, size_t N> class array_pair;   // forward declaration
	typedef EnetUtil::array_pair<uint8_t,NtwkUtilBufferSize> uint8_fixed_array_t;

	template<typename T, size_t N>
	class array_pair
	{
	public:
	    array_pair(void)		// Creates empty array with num_elements T objects
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
		    m_data_pair.first = new arrayUint8;
		    m_data_pair.second = 0;
	    }

	    array_pair(const array_pair<T,N>& obj)		// Copy constructor
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);

	    	if (!obj.is_valid())
	    	{
	    		// Make sure we create a valid (empty) project
	    		m_data_pair.first = new arrayUint8;
		    	*m_data_pair.first = *(obj.data());
			    m_data_pair.second = 0;
	    		return;
	    	}
	    	*m_data_pair.first = *(obj.data());			// Copy the obj array<>
		    m_data_pair.second = obj.num_elements();
	    }

	    array_pair(const arrayUint8 &objarray, size_t num_valid_array_elements)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (m_data_pair.first)
	    	{
	    		delete [] m_data_pair.first;
	    		m_data_pair.second = 0;
	    	}
	    	*m_data_pair.first = objarray;			// Copy the parameter array<>
		    m_data_pair.second = num_valid_array_elements;
	    }

	    // Destructor
	    virtual ~array_pair(void)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (m_data_pair.first)
	    	{
	    		delete [] m_data_pair.first;
	    	}
	    	m_data_pair.first = NULL;
    		m_data_pair.second = 0;
	    }

	    array_pair &operator=(array_pair<T,N> &obj)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);

	    	if (m_data_pair.first)
	    	{
	    		delete [] m_data_pair.first;
	    		m_data_pair.second = 0;
	    	}
	    	if (!obj.is_valid())
	    	{
	    		// Make sure we create a valid (empty) project
	    		m_data_pair.first = new arrayUint8;
		    	*m_data_pair.first = *(obj.data());
			    m_data_pair.second = 0;
	    		return *this;
	    	}
	    	*m_data_pair.first = *(obj.data());			// Copy the obj array<>
		    m_data_pair.second = obj.num_elements();
		    return *this;
	    }

		size_t num_elements() const                        	            { return m_data_pair.second; }
		const arrayUint8 *data(void) const				                { return m_data_pair.first; }
		bool is_empty() const							                { return num_elements() == 0; }
		bool is_valid() const 							                { return data() != NULL; }

	    // Gets a pointer to the pos'th element of the std::array<>
	    // Returns NULL if out of bounds or the object is invalid
		// ??? TODO: right now it's an assert()
	    T get_element(size_t pos) const
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	// TODO: exception?
	    	assert (is_valid() && pos < N);

	    	T val = (*m_data_pair.first)[pos];
	    	return val;
	    }

	    // Sets the pos'th element in the array to value
	    // returns false if the object is invalid or pos is out of bounds
	    // operator<T>=(T obj) has to be defined
	    bool set_element(size_t pos, const T& value)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (! is_valid() || pos >= N) return false;
	    	(*m_data_pair.first)[pos] = value;
			return true;
	    }

	private:
	    // Not allowed:
	    array_pair(array_pair<T,N> &&) = delete;			// Not movable
	    array_pair &operator=(array_pair<T,N> &&) = delete;	// Not movable


	private:
	    mutable std::mutex m_mutex;
		std::pair<arrayUint8 *, size_t> m_data_pair;
	}; // end of class array_pair definition


	class NtwkUtil
	{
	public:

		static int enetSend(Log::Logger& logger,
								int fd,
								EnetUtil::arrayUint8 & array_element_buffer,
								std::recursive_mutex& mutex = NtwkUtil::m_recursive_mutex,
								int flag = MSG_NOSIGNAL);

		static int enetReceive(Log::Logger& logger,
								int fd,
								EnetUtil::arrayUint8 & array_element_buffer,	// data and length
								size_t requestsize);

		static std::recursive_mutex m_recursive_mutex;
		static std::mutex m_mutex;
	};
} // namespace EnetUtil

