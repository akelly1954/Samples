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
	    const size_t num_elements(void)						{ return m_array_pair.num_elements(); }
	    const arrayUint8 *data(void) const					{ return m_array_pair.data(); }
	    uint8_fixed_array_t& get_array_pair()               { return m_array_pair; }
		bool is_empty() const								{ return num_elements() == 0; }
		bool is_valid() const 								{ return data() != NULL; }

	    // Gets a pointer to the pos'th element of the array
	    T get_element(size_t pos) const 					{ return m_array_pair.get_element(pos); }

	    // Sets the pos'th element in the array to value
	    // returns false if the object is invalid or pos is out of bounds
	    // operator<T>=(T obj) has to be defined
	    bool set_element(size_t pos, const T& value) { return m_array_pair.set_element(pos, value); }

	private:
	    // Not allowed:
	    fixed_size_array(fixed_size_array<T,N> &&) = delete;			// Not movable
	    fixed_size_array &operator=(fixed_size_array<T,N> &&) = delete;	// Not movable

	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(void) = default;	// Creates empty array with num_elements T objects

	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(fixed_size_array<T,N>& obj)		// Copy constructor
	    		: m_array_pair(obj.get_array_pair())  { }

	    // TODO:  Add constructor and ::create() call for
	    //        (const arrayUint8 &objarray, size_t num_valid_array_elements)

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

	public:
	    // Destructor
	    virtual ~fixed_size_array(void) = default;

        // This is the equivalent of the copy constructor
	    fixed_size_array<T,N>& operator=(fixed_size_array<T,N>& obj)
	    {
	    	m_array_pair = obj.get_array_pair();
	    	return *this;
	    }

		// Returns a shared_ptr to this object
	    std::shared_ptr<EnetUtil::fixed_size_array<T,N>> get_shared_ptr(void)
	    {
	    	return this->shared_from_this();
	    }

	private:
	    uint8_fixed_array_t m_array_pair;
	};
} // namespace EnetUtil

