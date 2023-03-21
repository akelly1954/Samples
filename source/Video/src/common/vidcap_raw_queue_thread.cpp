
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

#include <vidcap_queue_frame_workers.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_capture_thread.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <sys/wait.h>
#include <stdint.h>

using namespace VideoCapture;

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

// static members

std::mutex video_capture_queue::capture_queue_mutex;
bool video_capture_queue::s_terminated = false;
Util::condition_data<int> video_capture_queue::s_condvar(0);
Util::circular_buffer<Util::shared_ptr_uint8_data_t> video_capture_queue::s_ringbuf(100);

// pointers to all std::threads started by the raw queue object (this->)
std::vector<std::thread *> video_capture_queue::s_workerthreads;

// pointers to all frame worker objects started by the raw queue object (this->)
std::vector<frame_worker_thread_base *> video_capture_queue::s_workers;

//////////////////////////////////////////////////////////////////
// Please NOTE: Member functions for class video_capture_queue
// are at defined the end of the file
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// This is the main thread handler for the raw frame buffer queue.
//////////////////////////////////////////////////////////////////

void VideoCapture::raw_buffer_queue_handler()
{
    using namespace Video;
    using Util::Utility;
    using VideoCapture::video_capture_queue;

    FILE *filestream = NULL;        // if write_frames_to_file

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Waiting for kick-start...";

    if (video_capture_queue::s_terminated)
    {
        loggerp->info() << "VideoCapture::raw_buffer_queue_handler: Terminated before start of streaming...";
        return;
    }
    else
    {
        // Main is going to kick-start us to free this.
        // Wait for main() to signal us to start
        video_capture_queue::s_condvar.wait_for_ready();
    }

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Running.";

    // Set up the queue thread consumer objects needed in this run
    write2process_frame_worker *fw = new write2process_frame_worker(100);
    if (!Video::vcGlobals::write_frames_to_process)
    {
        delete fw;
        fw = nullptr;
    }
    else
    {
        // start the thread
        std::thread workerthread(&write2process_frame_worker::run, std::ref(*fw));
        workerthread.detach();

        video_capture_queue::register_worker_thread( &workerthread );
    }

    // This for loop starts all the registered worker threads and their queues
    for (auto itr = video_capture_queue::s_workers.begin();
                    itr != video_capture_queue::s_workers.end(); itr++)
    {
        if (! (*itr))
        {
            loggerp->error() << "VideoCapture::raw_buffer_queue_handler(): ERROR: null <worker*> object";
        }
        else
        {
            loggerp->debug() << "VideoCapture::raw_buffer_queue_handler(): found worker " <<
                                Utility::string_enquote((*itr)->m_label);
        }
    }

    // Captured frames also go to the output file only if the
    // option is set (which it is not, by default)
    if (Video::vcGlobals::write_frames_to_file)
    {
        filestream = create_output_file();
        if (filestream == NULL)
        {
            // detailed error message already emitted by the create function
            loggerp->error() << "Exiting...";
            video_capture_queue::set_terminated(true);
            return;
        }
        loggerp->debug() << "In VideoCapture::raw_buffer_queue_handler(): created " << Video::vcGlobals::output_file << ".";
    }

    /////////////////////////////////////////////////////////////////////
    while (!video_capture_queue::s_terminated)
    {
        video_capture_queue::s_condvar.wait_for_ready();

        while (!video_capture_queue::s_terminated && !video_capture_queue::s_ringbuf.empty())
        {
            // This shared_ptr serves all consumers of this particular video data buffer
            auto sp_frame = video_capture_queue::s_ringbuf.get();

            // Go through all the registered worker threads and add
            // the frame buffer to their queue.
            for (auto itr = video_capture_queue::s_workers.begin();
                            itr != video_capture_queue::s_workers.end(); itr++)
            {
                if (! (*itr))
                {
                    // TODO: Should this be an exception?
                    loggerp->error() << "VideoCapture::raw_buffer_queue_handler(): ERROR: null <worker*> object";
                }
                else
                {
                    // this call goes to the derived virtual worker object.
                    // The buffer (shared ptr to it) is simply added to its queue.
                    (*itr)->add_buffer_to_queue(sp_frame);
                    // and... that's it for this buffer.
                }
            }

            if (Video::vcGlobals::write_frames_to_file)
            {
                size_t nbytes = VideoCapture::write_frame_to_file(filestream, sp_frame);
                assert (nbytes == sp_frame->num_items());

                //////////////////////////////////////////////////////////////////////
                // Used in the code for DEBUG purposes only to simulate a heavy load.
                // Do not un-comment it lightly.
                // std::this_thread::sleep_for(std::chrono::milliseconds(40));
                //////////////////////////////////////////////////////////////////////
            }
        }
    }

    loggerp->debug() << "Queue thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!video_capture_queue::s_ringbuf.empty())
    {
        auto sp_frame = video_capture_queue::s_ringbuf.get();
        loggerp->debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_items() << " bytes ";
        if (Video::vcGlobals::write_frames_to_file)
        {
            size_t nbytes = VideoCapture::write_frame_to_file(filestream, sp_frame);
            assert (nbytes == sp_frame->num_items());

            //////////////////////////////////////////////////////////////////////
            // Used in the code for DEBUG purposes only to simulate a heavy load.
            // Do not un-comment it lightly.
            // std::this_thread::sleep_for(std::chrono::milliseconds(40));
            //////////////////////////////////////////////////////////////////////
        }
    }
    if (Video::vcGlobals::write_frames_to_file && filestream != NULL)
    {
        fflush(filestream);
        fclose(filestream);
    }
}

// Open/truncate the output file that will hold captured frames
FILE * VideoCapture::create_output_file()
{
    using Util::Utility;

    int errnocopy = 0;
    FILE *output_stream = NULL;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if ((output_stream = ::fopen (Video::vcGlobals::output_file.c_str(), "w+")) == NULL)
    {
        errnocopy = errno;
        loggerp->error() << "Cannot create/truncate output file \"" <<
        Video::vcGlobals::output_file << "\": " << Utility::get_errno_message(errnocopy);
    }
    else
    {
        loggerp->debug() << "Created/truncated output file \"" << Video::vcGlobals::output_file << "\"";
    }
    return output_stream;
}

size_t VideoCapture::write_frame_to_file(FILE *filestream, Util::shared_ptr_uint8_data_t sp_frame)
{
    using Util::Utility;

    size_t elementswritten = std::fwrite(sp_frame->_begin(), sizeof(uint8_t), sp_frame->num_items(), filestream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if (byteswritten != sp_frame->num_items())
    {
        loggerp->error() << "VideoCapture::write_frame_to_file: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_items() << ", got " << byteswritten << " bytes: " <<
                        Utility::get_errno_message(errnocopy);
    }
    fflush(filestream);
    return byteswritten;
}

//////////////////////////////////////////////////////////////////
// Member functions for class video_capture_queue
//////////////////////////////////////////////////////////////////

video_capture_queue::~video_capture_queue()
{
    for (auto& witr : video_capture_queue::s_workers)
    {
        if (witr != nullptr)
        {
            delete witr;
            witr = nullptr;
        }
    }
}

void video_capture_queue::set_terminated(bool t)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

    video_capture_queue::s_terminated = t;

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
}

// Note: this method runs on a different thread than the other methods in this object.
// It's called from the specific video raw capture driver on its thread.
void video_capture_queue::add_buffer_to_raw_queue(void *p, size_t bsize)
{
    using namespace Util;

    if (p != NULL)
    {
        uint8_t *up = static_cast<uint8_t*>(p);
        auto sp = shared_uint8_data_t::create(up, bsize);
        VideoCapture::video_capture_queue::s_ringbuf.put(sp, VideoCapture::video_capture_queue::s_condvar);
    }
}

void video_capture_queue::register_worker_thread(std::thread *workerthread)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);
    s_workerthreads.push_back(workerthread);
}

void video_capture_queue::register_worker(frame_worker_thread_base *worker)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);
    s_workers.push_back(worker);
}
