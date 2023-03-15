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
        : m_numitems(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_numitems = num_items;
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
        : m_numitems(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_numitems = obj.num_items();
        mp_data = new T[m_numitems];

        // copy from _begin() to (not including) _end, to data().
        std::copy(obj._begin(), obj._end(), data());
    }

    // Raw data constructor (i.e. video frame from driver)
    data_item_container(T* rawdata, size_t nelements)
        : m_numitems(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_numitems = nelements;
        mp_data = new T[nelements];

        // copy from beginning to (not including) the end, to data().
        if (nelements > 0) std::copy(rawdata, rawdata + nelements, data());
    }

    // Move constructor
    data_item_container(data_item_container&& obj)
        : m_numitems(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        mp_data = obj.data();
        m_numitems = obj.num_items();
        obj.set_invalid();
    }

    // Copy = (assignment)
    data_item_container& operator=(data_item_container& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (this != &obj)
        {
            if (mp_data != nullptr) delete[] mp_data;
            m_numitems = obj.num_items();
            mp_data = new T[m_numitems];
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
            m_numitems = obj.num_items();
            obj.set_invalid();
        }
        return *this;
    }

    void set_invalid()
    {
        m_numitems = 0;
        mp_data = nullptr;
    }

    bool is_valid()
    {
        return m_numitems > 0 && mp_data != nullptr;
    }

    size_t num_items()
    {
        return m_numitems;
    }

    size_t bytelength()
    {
        return m_numitems * sizeof(mp_data[0]);
    }

    T* data()       { return mp_data; }
    T* _begin()     { return mp_data; }
    T* _end()       { return _begin() + num_items(); }

private:
    size_t m_numitems;
    T *mp_data;
    mutable std::mutex m_mutex;
};

////////////////////////////////////////////////////////////////////////////
// Template class shared_data_items definition
////////////////////////////////////////////////////////////////////////////

template<typename T> class shared_data_items;   // forward declaration

typedef     Util::shared_data_items<uint8_t>          shared_uint8_data_t;
typedef     std::shared_ptr<shared_uint8_data_t>      shared_ptr_uint8_data_t;

// data_item_container
template<typename T>
class shared_data_items : public std::enable_shared_from_this<shared_data_items<T>>
{
public:
    // constructors and destructor are defined further down below
    data_item_container<T> * get_data_item_container(void)
    {
        return p_shared_data;
    }

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
    // The following sections mostly involve creation and construction, and deletion.
    // First, the "private" sections (methods that are disallowed, as well as
    // methods that are prevented from being called by std::make_shared<>):
    ////////////////////////////////////////////////////////////////////////////

private:
    // Not allowed:
    shared_data_items(shared_data_items<T> &&) = delete;            // Not movable
    shared_data_items &operator=(shared_data_items<T> &&) = delete; // Not movable

private:
    // private in order to prevent make_shared<> from being called
    // (see create() functions below).
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
    // (see create() functions below)
    shared_data_items(shared_data_items<T>& obj)        // Copy constructor
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // this data_item_container constructor copies the data:
        p_shared_data = new data_item_container<T>(*obj.get_data_item_container());
    }

    // private in order to prevent make_shared<> from being called
    // (see create() functions below)
    shared_data_items(data_item_container<T>& dobj)
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // this data_item_container constructor copies the data:
        p_shared_data = new data_item_container<T>(dobj);
    }

    // private in order to prevent make_shared<> from being called
    // (see create() functions below)
    shared_data_items(T *databuffer, size_t nelements)
        : p_shared_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // this data_item_container constructor copies the data:
        p_shared_data = new data_item_container<T>(databuffer, nelements);
    }

    ////////////////////////////////////////////////////////////////////////////
    // The following is the "public" section of object creation, construction, and deletion.
    ////////////////////////////////////////////////////////////////////////////

public:
    // The first time this object is constructed in order to obtain a shared_ptr<> for a brand
    // new managed object, the ::create() function MUST be called.  All subsequent shared_ptr<>'s
    // that are to be made for this instance of the object, MUST be obtained by calling the
    // ::get_shared_ptr() member function.  So - call ::create() once, and then ::get_shared_ptr()
    // as many times as needed to obtain a shared_ptr<> for this object.
    //
    // See also private constructors above (placed there to prevent make_shared<> from being called.

    // This method HAS to be called the first time the object is created (instead of
    // the equivalent constructor). Subsequent shared_ptr<>'s can be had by calling the
    // get_shared_ptr() method declared/defined below.
    [[nodiscard]] static std::shared_ptr<Util::shared_data_items<T>> create(size_t numitems = 0)
    {
        return std::shared_ptr<Util::shared_data_items<T>>(new Util::shared_data_items<T>(numitems));
    }

    // This method HAS to be called the first time the object is created (instead of
    // the equivalent constructor). Subsequent shared_ptr<>'s can be had by calling the
    // get_shared_ptr() method declared/defined below.
    [[nodiscard]] static std::shared_ptr<Util::shared_data_items<T>> create(shared_data_items<T>& obj)
    {
        return std::shared_ptr<Util::shared_data_items<T>>(new Util::shared_data_items<T>(obj));
    }

    // This method HAS to be called the first time the object is created (instead of
    // the equivalent constructor). Subsequent shared_ptr<>'s can be had by calling the
    // get_shared_ptr() method declared/defined below.
    [[nodiscard]] static std::shared_ptr<Util::shared_data_items<T>> create(data_item_container<T>& dobj)
    {
        return std::shared_ptr<Util::shared_data_items<T>>(new Util::shared_data_items<T>(dobj));
    }

    // This method HAS to be called the first time the object is created (instead of
    // the equivalent constructor). Subsequent shared_ptr<>'s can be had by calling the
    // get_shared_ptr() method declared/defined below.
    [[nodiscard]] static std::shared_ptr<Util::shared_data_items<T>> create(T *databuffer, size_t nelements)
    {
        return std::shared_ptr<Util::shared_data_items<T>>(new Util::shared_data_items<T>(databuffer, nelements));
    }

public:
    // Destructor
    virtual ~shared_data_items()
    {
        if (p_shared_data != nullptr) delete p_shared_data;
    }

    // copy assignment operator of the encapsulating object
    shared_data_items<T>& operator=(shared_data_items<T>& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (p_shared_data) delete p_shared_data;

        // this data_item_container constructor copies the data:
        p_shared_data = new data_item_container<T>(obj);
        return *this;
    }

    // copy assignment operator of the embedded data item
    shared_data_items<T>& operator=(data_item_container<T>& dobj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (p_shared_data) delete p_shared_data;

        // this data_item_container constructor copies the data:
        p_shared_data = new data_item_container<T>(dobj);
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////
    // END OF THE "public" section of object creation, construction, and deletion.
    ////////////////////////////////////////////////////////////////////////////

    // And finally, get a shared_ptr<> to this object...
    std::shared_ptr<Util::shared_data_items<T>> get_shared_ptr(void)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return this->shared_from_this();
    }

private:
    mutable std::mutex m_mutex;
    data_item_container<T> *p_shared_data;
};

} // end of namespace Util

