#pragma once

#include <circular_buffer.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <NtwkUtil.hpp>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

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

	// REFERENCES:
	// 		https://en.cppreference.com/w/cpp/memory/enable_shared_from_this
	//		https://embeddedartistry.com/blog/2017/01/11/stdshared_ptr-and-shared_from_this/  --  AND
	//		https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/shared_ptr.cpp

	template<typename T, size_t N>
	class fixed_size_array : public std::enable_shared_from_this<fixed_size_array<T,N>>
		{
	private:
	    size_t m_num_elements = N;
	public:
	    size_t numElements(void) const 				{ return m_num_elements; }
	    const std::array<T,N> *data(void) const		{ return p_fixed_array; }

	private:
	    // Not allowed:
	    fixed_size_array(fixed_size_array<T,N> &&) = delete;			// Not movable
	    fixed_size_array &operator=(fixed_size_array<T,N> &&) = delete;	// Not movable

	private:
	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(void)		// Creates empty array with num_elements T objects
	    :
	    	m_isvalid(false),
			p_fixed_array(NULL)
	    {
		    if (m_num_elements < 1)
		    {
		    	// valid stays false, and pointer to array is NULL.
				return;
		    }
		    p_fixed_array = new std::array<T,N>();
		    m_isvalid = true;
	    }

	    // private in order to prevent make_shared<> from being called
	    // (see static create() functions below)
	    fixed_size_array(const fixed_size_array<T,N>& obj)		// Copy constructor
		:
			m_isvalid(false),
			m_num_elements(N),
			p_fixed_array(NULL)
	    {
	    	// valid stays false, and pointer to array is NULL.
	    	if (!obj.isValid())
	    	{
	    		return;
	    	}

	    	// TODO: Revisit copy-constructor logic after ::create() is done

	    	p_fixed_array = new std::array<T,N>;
	    	*p_fixed_array = *(obj.data()); 			// Copy the obj array<>
		    m_isvalid = true;
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
	    //		 std::shared_ptr<fixed_size_array<T,N> spFSArray = fixed_size_array<T,N>::create(1234);
	    //
	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(void)
	    {
	            return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>());
	    }

	    // Use this version of ::create() to get a new shared_ptr<> managing a brand new T object which
	    // is a copy of the data managed by the parameter object being copied. Like a copy constructor,
	    // except that the new T object is managed by a new shared_ptr<>, with no connection to the
	    // copied object.
	    //
	    //		 std::shared_ptr<fixed_size_array<T,N>> spFSArray =
	    //					fixed_size_array<T,N>::create(fixed_size_array<T,N> &obj);
	    //
	    [[nodiscard]] static std::shared_ptr<EnetUtil::fixed_size_array<T,N>> create(const EnetUtil::fixed_size_array<T,N> &obj)
	    {
	            return std::shared_ptr<EnetUtil::fixed_size_array<T,N>>(new EnetUtil::fixed_size_array<T,N>(obj));
	    }

	public:
	    // Destructor
	    virtual ~fixed_size_array(void)
	    {
	    	if (p_fixed_array)
	    	{
	    		delete [] p_fixed_array;
	    	}
	    }

	    fixed_size_array<T,N>& operator=(fixed_size_array<T,N> const &obj)
	    {
	    	m_isvalid = obj.isValid();
	    	// this copies the parameter object's array object.
	    	p_fixed_array = *(obj.data());
	    	return *this;
	    }

	    bool isValid(void) const 		{ return m_isvalid; }

		// Returns a shared_ptr to this object
	    std::shared_ptr<EnetUtil::fixed_size_array<T,N>> get_shared_ptr(void)
	    {
	    	return this->shared_from_this();
	    }

	private:
	    bool m_isvalid;
	    std::array<T,N>* p_fixed_array;
	};
} // namespace EnetUtil

