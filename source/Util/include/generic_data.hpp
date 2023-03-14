#pragma once

/////////////////////////////////////////////////////////////////////////////////
// MIT License
//
// Copyright (c) 2023 Andrew Kelly
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
#include <circular_buffer.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <mutex>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <ostream>
#include <assert.h>

namespace Util
{

////////////////////////////////////////////////////////////////////////////
// Template class data_item_container definition
////////////////////////////////////////////////////////////////////////////

template<typename T>
class data_item_container
{
public:
    // Default constructor creates an empty object or a
    // valid object (with at least one member).
    data_item_container(size_t num_items = 0)
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_numitemes = num_items;
        mp_data = new T[num_items];
    }

    // Destructor
    virtual ~data_item_container()
    {
        if (mp_data != nullptr)
        {
            delete[] mp_data;
            mp_data = nullptr;
        }
    }

    // Copy constructor
    data_item_container(data_item_container& obj)
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_numitemes = obj.num_items();
        mp_data = new T[m_numitemes];

        // copy from _begin() to (not including) _end, to data().
        std::copy(obj._begin(), obj._end(), data());
    }

    // Raw data constructor (i.e. video frame from driver)
    data_item_container(T* rawdata, size_t nelements)
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_numitemes = nelements;
        mp_data = new T[nelements];

        // copy from beginning to (not including) the end, to data().
        if (nelements > 0) std::copy(rawdata, rawdata + nelements, data());
    }

    // Move constructor
    data_item_container(data_item_container&& obj)
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        mp_data = obj.data();
        m_numitemes = obj.num_items();
        obj.set_invalid();
    }

    // Copy = (assignment)
    data_item_container& operator=(data_item_container& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (this != &obj)
        {
            if (mp_data != nullptr) delete[] mp_data;
            m_numitemes = obj.num_items();
            mp_data = new T[m_numitemes];
            // copy from _begin() to (not including) _end, to data().
            std::copy( obj._begin(), obj._end(), data());
        }
        return *this;
    }

    // Move = (operator)
    data_item_container& operator=(data_item_container&& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (this != &obj)
        {
            if (mp_data != nullptr) delete[] mp_data;
            mp_data = obj.data();
            m_numitemes = obj.num_items();
            obj.set_invalid();
        }
        return *this;
    }

    void set_invalid()
    {
        m_numitemes = 0;
        mp_data = nullptr;
    }

    bool is_valid()
    {
        return m_numitemes > 0 && mp_data != nullptr;
    }

    size_t num_items()
    {
        return m_numitemes;
    }

    size_t bytelength()
    {
        return m_numitemes * sizeof(mp_data[0]);
    }

    T* data()       { return mp_data; }
    T* _begin()     { return mp_data; }
    T* _end()       { return _begin() + num_items(); }

private:
    size_t m_numitemes;
    T *mp_data;
    mutable std::mutex m_mutex;
};

////////////////////////////////////////////////////////////////////////////
// Template class shared_data_items definition
////////////////////////////////////////////////////////////////////////////

template<typename T> class shared_data_items;   // forward declaration
typedef Util::shared_data_items<uint8_t> shared_uint8_data;

// data_item_container
template<typename T>
class shared_data_items : public std::enable_shared_from_this<data_item_container<T>>
{
public:
    // constructors and destructor are defined further down below
    T* data(void)
    {
        if(p_shared_data) return p_shared_data->data();
        return nullptr;
    }

    T* _begin(void)
    {
        if(p_shared_data) return p_shared_data->_begin();
        return nullptr;
    }

    T* _end(void)
    {
        if(p_shared_data) return p_shared_data->_end();
        return nullptr;
    }
    bool is_valid(void)
    {
        if(p_shared_data) return p_shared_data->is_valid();
        return false;
    }

    bool is_empty(void)
    {
        if(p_shared_data) return p_shared_data->num_items() == 0;
        return true;  // true, meaning - yes, it's empty... (and invalid)
    }

    const size_t num_items(void)
    {
        if(p_shared_data) return p_shared_data->num_items();
        return 0;
    }

    T& operator[](size_t n)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (p_shared_data == nullptr || p_shared_data->data() == nullptr)
        {
            std::stringstream ostr;
            ostr << "shared_data_items: object is empty (requested position " << std::to_string(n) << ").";
            throw std::runtime_error(ostr.str());
        }
        if (n >= num_items())
        {
            std::stringstream ostr;
            ostr << "shared_data_items: out of bounds " << std::to_string(n) << " (out of "
                 << std::to_string(num_items()) << ").";
            throw std::runtime_error(ostr.str());
        }
        return (p_shared_data->data())[n];
    }

    ////////////////////////////////////////////////////////////////////////////
    // The following sections mostly involve creation, construction, and deletion.
    // First, the "private" sections (methods that are disallowed, as well as
    // methods that are prevented from being called by std::make_shared<>):
    ////////////////////////////////////////////////////////////////////////////

private:
    // Not allowed:
    shared_data_items(shared_data_items<T> &&) = delete;            // Not movable
    shared_data_items &operator=(shared_data_items<T> &&) = delete; // Not movable

private:
    // private in order to prevent make_shared<> from being called
    // (see static create() functions below).
    // This private constructor creates either an empty (and invalid)
    // object, or a valid one, with at least one T object.
    shared_data_items(size_t numitems = 0)
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // if numitems is 0, this creates an empty and
        // invalid object, which is what we want.
        p_shared_data = new data_item_container<T>(numitems);
    }

    // private in order to prevent make_shared<> from being called
    // (see static create() functions below)
    shared_data_items(shared_data_items<T>& obj)        // Copy constructor
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (p_shared_data) delete p_shared_data;
        p_shared_data = new data_item_container<T>(obj.num_items());
        *p_shared_data = obj;
    }

    // private in order to prevent make_shared<> from being called
    // (see static create() functions below)
    shared_data_items(data_item_container<T>& dobj)
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        p_shared_data = new data_item_container<T>(dobj);
    }

    // private in order to prevent make_shared<> from being called
    // (see static create() functions below)
    shared_data_items(T *databuffer, size_t nelements)
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        p_shared_data = new data_item_container<T>(databuffer, nelements);
    }

private:
    mutable std::mutex m_mutex;
    data_item_container<T> *p_shared_data;
    size_t m_numitems;
};

} // end of namespace Util

