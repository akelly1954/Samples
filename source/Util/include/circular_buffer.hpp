#pragma once

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Adapted from:
//
//     https://github.com/embeddedartistry/embedded-resources/blob/master/examples/cpp/circular_buffer.cpp
//
// The circular_buffer is also known as a ring buffer or bounded_buffer - a fixed-size buffer
// that work as if the memory is contiguous & circular in nature.
//
// As memory is generated and consumed, data does not need to be re-shuffled - rather, the head/tail
// pointers are adjusted. When data is added, the head pointer advances. When data is consumed, the
// tail pointer advances. If you reach the end of the buffer, the pointers simply wrap around
// to the beginning.
//
// Reference: https://en.wikipedia.org/wiki/Circular_buffer
//
// For example of use, see the main program in main_programs/main_circular_buffer.cpp
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <condition_data.hpp>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <deque>
#include <string>

namespace Util {

template<class T>
class circular_buffer {
public:
    explicit circular_buffer(size_t size) :
            buf_(std::unique_ptr<T[]>(new T[size])), max_size_(size) {

    }

    void put(T item) {
        std::lock_guard<std::mutex> lock(mutex_);

        buf_[head_] = item;

        if (full_) {
            tail_ = (tail_ + 1) % max_size_;
        }

        head_ = (head_ + 1) % max_size_;

        full_ = head_ == tail_;
    }

    // Call put() with a condition_data mechanism to
    // notify a waiting thread that there is data ready.
    void put(T item, condition_data<int>& condvar) {
    	put(item);
    	condvar.send_ready (size(), condition_data<int>::All);
    }

    T get() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (empty()) {
            return T();
        }

        //Read data and advance the tail (we now have a free space)
        auto val = buf_[tail_];
        full_ = false;
        tail_ = (tail_ + 1) % max_size_;

        return val;
    }

    ///////////////////////////////////////////////////
    // FOR DEBUG PURPOSES ONLY!!!
    // T *getptr() const
    // {
    //     return buf_.get();
    // }
    // size_t gethead() { return head_; }
    // size_t gettail() { return tail_; }
    ///////////////////////////////////////////////////

    // Gets a copy of the "latest" n members of the buffer into the result deque (latest first).
    // If n is 0, all members are copied.
    // If n is greater than the size of the buffer, only that size number of members is returned.
    // The first member of the deque will be the latest member added to buffer, the next will be
    // the one before that, etc.
    void get_members_in_deque(std::deque<T>& result, size_t num = 0) const {
        std::lock_guard<std::mutex> lock(mutex_);
        result.clear();
        if (empty())
            return;

        int avail = size();
        size_t n = num;
        if (n > avail || n == 0)
            n = avail;

        size_t cur = 0;
        for (size_t i = 0; i < n; i++) {
            cur = (tail_ + avail - i - 1) % avail;
            result.push_back(buf_[cur]);
        }
    }

    void reset() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
        full_ = false;
    }

    bool empty() const {
        //if head and tail are equal, we are empty
        return (!full_ && (head_ == tail_));
    }

    bool full() const {
        //If tail is ahead the head by 1, we are full
        return full_;
    }

    size_t capacity() const {
        return max_size_;
    }

    size_t size() const {
        size_t size = max_size_;

        if (!full_) {
            if (head_ >= tail_) {
                size = head_ - tail_;
            } else {
                size = max_size_ + head_ - tail_;
            }
        }

        return size;
    }

private:
    mutable std::mutex mutex_;
    std::unique_ptr<T[]> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    const size_t max_size_;
    bool full_ = 0;
};

} // namespace Util

