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
#include <assert.h>

namespace Util
{

template<typename T>
class data_item_container
{
public:
    // Default constructor creates an empty object or a
    // valid object (with at least one member).
    data_item_container(size_t numItems = 0)
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        if (numItems == 0) return;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_numitemes = numItems;
        mp_data = new T[numItems];
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

        m_numitemes = obj.numItems();
        mp_data = new T[m_numitemes];

        // copy from _begin() to (not including) _end, to data().
        std::copy(obj._begin(), obj._end(), data());
    }

    // Move constructor
    data_item_container(data_item_container&& obj) noexcept
        : m_numitemes(0)
        , mp_data(nullptr)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        mp_data = obj.data();
        m_numitemes = obj.numItems();
        obj.setInvalid();
    }

    // Copy = (assignment)
    data_item_container& operator=(data_item_container& obj)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (this != &obj)
        {
            if (mp_data != nullptr) delete[] mp_data;
            m_numitemes = obj.numItems();
            mp_data = new T[m_numitemes];
            // copy from _begin() to (not including) _end, to data().
            std::copy( obj._begin(), obj._end(), data());
        }
        return *this;
    }

    // Move = (operator)
    data_item_container& operator=(data_item_container&& obj) noexcept
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (this != &obj)
        {
            if (mp_data != nullptr) delete[] mp_data;
            mp_data = obj.data();
            m_numitemes = obj.numItems();
            obj.setInvalid();
        }
        return *this;
    }

    void setInvalid()
    {
        m_numitemes = 0;
        mp_data = nullptr;
    }

    bool isValid()
    {
        return m_numitemes > 0 && mp_data != nullptr;
    }

    size_t numItems()
    {
        return m_numitemes;
    }

    size_t bytelength()
    {
        return m_numitemes * sizeof(mp_data[0]);
    }

    T* data()   { return mp_data; }
    T* _begin()  { return mp_data; }
    T* _end()   { return _begin() + numItems(); }

private:
    size_t m_numitemes;
    T *mp_data;
    mutable std::mutex m_mutex;
};

} // end of namespace Util

