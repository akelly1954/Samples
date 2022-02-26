#pragma once

#include <circular_buffer.hpp>
#include <NtwkUtil.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
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

	// TODO:  There's no reason for this object to be tied down to size_t/uint8 for
	// std::array<>.  This needs to be cleaned up and expanded to other types (including
	// helper objects for managing things.
	template<typename T, size_t N> class fixed_size_array;   // forward declaration
	typedef EnetUtil::fixed_size_array<uint8_t,NtwkUtilBufferSize> fixed_uint8_array_t;

	// REFERENCES:
	// 		https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
	//		https://embeddedartistry.com/blog/2017/01/11/stdshared_ptr-and-shared_from_this/  --  AND
	//		https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/shared_ptr.cpp

	template<typename T, size_t N>
	class fixed_size_array : public std::enable_shared_from_this<fixed_size_array<T,N>>
	{
	public:
	    const size_t num_valid_elements(void)				{ return m_num_valid_elements; }
	    const arrayUint8 data(void) const					{ return m_array_data; }
		bool is_empty()										{ return num_valid_elements() == 0; }
		bool has_data()										{ return num_valid_elements() > 0; }

	    // Gets the value of the pos'th element of the std::array<>
	    // TODO: returns false if pos is out of bounds. This might
		// have to throw and exception instead.
	    bool get_element(size_t pos, T& target)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (pos >= N) return false;
	    	target = m_array_data[pos];
	    	return true;
	    }

	    // The number of valid elements is managed by the external
	    // using code - completely.  When this function is called,
	    // it's because set_element() was called, for example.
	    bool set_num_valid_elements(size_t num)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (num >= N) return false;
	    	m_num_valid_elements = num;
	    	return true;
	    }

	    // Sets the pos'th element in the array to value.
	    // Returns false if pos is out of bounds.
	    bool set_element(size_t pos, const T& value)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	if (pos >= N) return false;
	    	m_array_data[pos] = value;
			return true;
	    }

	private:
	    // Not allowed:
	    fixed_size_array(fixed_size_array<T,N> &&) = delete;			// Not movable
	    fixed_size_array &operator=(fixed_size_array<T,N> &&) = delete;	// Not movable

	private:
	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(void)     	// Creates empty array with num_valid_elements T objects
				: m_num_valid_elements(0)
		{
	    	std::lock_guard<std::mutex> lock(m_mutex);
			m_num_valid_elements = 0;
		}

	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(fixed_size_array<T,N>& obj)		// Copy constructor
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	m_array_data = obj.data();
	    	m_num_valid_elements = obj.num_valid_elements();
	    }

	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(datapairUint8_t& obj)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	m_num_valid_elements = (obj.first > m_array_data.size()? m_array_data.size() : obj.first);
	    	for (size_t i = 0; i < m_num_valid_elements; i++)
	    	{
	    		m_array_data[i] = obj.second[i];
	    	}
	    }

	public:
	    // The first time this object is constructed in order to obtain a shared_ptr<> for a brand
	    // new managed object, the ::create() function MUST be called.  All subsequent shared_ptr<>'s
	    // that are to be made for this instance of the object, MUST be obtained by calling the
	    // ::get_shared_ptr() member function.  So, call ::create once, and then ::get_shared_ptr()
	    // as many times as needed.
	    //
	    // See also private contstuctors above (placed there to prevent make_shared<> from being called.

	    // Use this version of ::create() for an array which has a specific number of elements (1234)
	    // in place of a constructor like so:
	    //
	    //		 std::shared_ptr<fixed_size_array<T,1234> spFSArray = fixed_size_array<T,N>::create();
	    //
	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(void)
	    {
	    	return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>());
	    }

	    // Use this version of ::create() to get a new shared_ptr<> managing a brand new T object which
	    // is a copy of the data managed by the parameter object being copied. A new shared_ptr<> is 
        // created to manage the copy.  
	    //
	    //		 std::shared_ptr<fixed_size_array<T,N>> spFSArray =
	    //					fixed_size_array<T,N>::create(fixed_size_array<T,N> &obj);
	    //
	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(EnetUtil::fixed_size_array<T,N> &obj)
	    {
            return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>(obj));
	    }

	    // same story: this create call will call the equivalent fixed_array constructor.
	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(datapairUint8_t& obj)
	    {
	    	return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>(obj));
	    }

	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(arrayUint8& obj, size_t nvalid_elements)
	    {
	    	datapairUint8_t newpair(nvalid_elements, obj);
	    	return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>(newpair));
	    }

	public:
	    // Destructor
	    virtual ~fixed_size_array(void) = default;

        // This is the equivalent of the copy constructor
	    fixed_size_array<T,N>& operator=(fixed_size_array<T,N>& obj)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	m_array_data = obj.data();
	    	m_num_valid_elements = obj.num_valid_elements();
	    	return *this;
	    }

	    fixed_size_array<T,N>& operator=(datapairUint8_t& obj)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	m_num_valid_elements = (obj.first > m_array_data.size()? m_array_data.size() : obj.first);
	    	for (size_t i = 0; i < m_num_valid_elements; i++)
	    	{
	    		m_array_data[i] = obj.second[i];
	    	}
	    	return *this;
	    }

		// Returns a shared_ptr to this object
	    std::shared_ptr<EnetUtil::fixed_size_array<T,N>> get_shared_ptr(void)
	    {
	    	std::lock_guard<std::mutex> lock(m_mutex);
	    	return this->shared_from_this();
	    }

	private:
	    mutable std::mutex m_mutex;
	    arrayUint8 m_array_data;
	    size_t m_num_valid_elements;
	};
} // namespace EnetUtil

