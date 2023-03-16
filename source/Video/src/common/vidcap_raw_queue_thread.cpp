
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

#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_capture_thread.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
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

void VideoCapture::raw_buffer_queue_handler()
{
    using namespace Video;
    using Util::Utility;

    FILE *filestream = NULL;        // if write_frames_to_file
    FILE *processstream = NULL;     // if write_frames_to_process

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Waiting for kick-start...";

    {
        std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

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
    }

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Running.";

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
    // Captured frames also go to the output file only if the
    // option is set (which it is not, by default)
    if (Video::vcGlobals::write_frames_to_process)
    {
        processstream = create_output_process();
        if (processstream == NULL)
        {
            // detailed error message already emitted by the create function
            loggerp->error() << "Exiting...";
            video_capture_queue::set_terminated(true);
            return;
        }
        loggerp->debug() << "In VideoCapture::raw_buffer_queue_handler(): Successfully started \"" << Video::vcGlobals::output_process << "\".";
    }

    while (!video_capture_queue::s_terminated)
    {
        // loggerp->debug() << "In VideoCapture::raw_buffer_queue_handler(): not terminated, waiting on condvar 1";
        video_capture_queue::s_condvar.wait_for_ready();
        // loggerp->debug() << "In VideoCapture::raw_buffer_queue_handler(): kick started, looping";

        while (!video_capture_queue::s_terminated && !video_capture_queue::s_ringbuf.empty())
        {
            // This shared_ptr serves all consumers of this particular video data buffer
            auto sp_frame = video_capture_queue::s_ringbuf.get();

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

            if (Video::vcGlobals::write_frames_to_process)
            {
                size_t nbytes = VideoCapture::write_frame_to_process(processstream, sp_frame);
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

        if (Video::vcGlobals::write_frames_to_process)
        {
            size_t nbytes = VideoCapture::write_frame_to_process(processstream, sp_frame);
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
    if (Video::vcGlobals::write_frames_to_process && processstream != NULL)
    {
        loggerp->debug() << "Shutting down the process \""
                       << Video::vcGlobals::output_process << "\" (fflush, pclose()): ";
        fflush(processstream);
        int errnocopy = 0;
        if (::pclose(processstream) == -1)
        {
            errnocopy = errno;
            loggerp->error() << "Error shutting down the process \""
                              << Video::vcGlobals::output_process << "\" on pclose(): "
                              << Utility::get_errno_message(errnocopy);
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

// Start up the process that will receive video frames in it's std input
FILE * VideoCapture::create_output_process()
{
    using Util::Utility;

    int errnocopy = 0;
    FILE *output_stream = NULL;

    auto loggerp = Util::UtilLogger::getLoggerPtr();
    VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

    // Need to refresh output process since command line options
    // may have changed the requested pixel-format.
    Json::Value& cfg_root = Config::ConfigSingleton::GetJsonRootCopyRef();

    std::string actual_process = ifptr->get_popen_process_string();

    loggerp->debug() << "create_output_process: Starting output process:  " << actual_process;

    if ((output_stream = ::popen (actual_process.c_str(), "w")) == NULL)
    {
        errnocopy = errno;
        loggerp->error() << "create_output_process: Could not start the process \"" << actual_process
                       << "\": " << Utility::get_errno_message(errnocopy);
    }
    else
    {
        loggerp->debug() << "create_output_process: Started the process \"" << actual_process << "\".";
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

size_t VideoCapture::write_frame_to_process(FILE *processstream, Util::shared_ptr_uint8_data_t sp_frame)
{
    using Util::Utility;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    size_t elementswritten = std::fwrite(sp_frame->_begin(), sizeof(uint8_t), sp_frame->num_items(), processstream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_items())
    {
        loggerp->error() << "VideoCapture::write_frame_to_process: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_items() << ", got " << byteswritten << " bytes: " <<
                        Utility::get_errno_message(errnocopy);
    }
    fflush(processstream);
    return byteswritten;
}


