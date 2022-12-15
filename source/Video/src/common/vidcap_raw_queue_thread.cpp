#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_profiler_thread.hpp>
#include <video_capture_globals.hpp>
#include <ConfigSingleton.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
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

using namespace VideoCapture;

/////////////////////////////////////////////////////////////////
// NOTE: This is a different thread to the main thread.
/////////////////////////////////////////////////////////////////

// static members

std::mutex video_capture_queue::capture_queue_mutex;
bool video_capture_queue::s_terminated = false;
Util::condition_data<int> video_capture_queue::s_condvar(0);
Util::circular_buffer<std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>>>
                                                                            video_capture_queue::s_ringbuf(100);

void VideoCapture::raw_buffer_queue_handler()
{
    auto loggerp = Util::UtilLogger::getLoggerPtr();

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Waiting for kick-start...";

    {
        std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

        if (!video_capture_queue::s_terminated)
        {
            // Main is going to kick-start us to free this.
            // Wait for main() to signal us to start
            video_capture_queue::s_condvar.wait_for_ready();
        }
    }

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Running.";

    loggerp->debug() << "VideoCapture::raw_buffer_queue_handler: Terminating...";
    video_capture_queue::set_terminated(true);
}

#if 0
void VideoCapture::raw_buffer_queue_handler(Log::Logger logger)
{
    using namespace Video;
    FILE *filestream = NULL;        // if write_frames_to_file
    FILE *processstream = NULL;     // if write_frames_to_process

    // Captured frames also go to the output file only if the
    // option is set (which it is not, by default)
    if (Video::vcGlobals::write_frames_to_file)
    {
        filestream = create_output_file(logger);
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
        processstream = create_output_process(logger);
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
                size_t nbytes = VideoCapture::write_frame_to_file(logger, filestream, sp_frame);
                assert (nbytes == sp_frame->num_valid_elements());

                //////////////////////////////////////////////////////////////////////
                // Used in the code for DEBUG purposes only to simulate a heavy load.
                // Do not un-comment it lightly.
                // std::this_thread::sleep_for(std::chrono::milliseconds(40));
                //////////////////////////////////////////////////////////////////////
            }

            if (Video::vcGlobals::write_frames_to_process)
            {
                size_t nbytes = VideoCapture::write_frame_to_process(logger, processstream, sp_frame);
                assert (nbytes == sp_frame->num_valid_elements());

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
        loggerp->debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_valid_elements() << " bytes ";
        if (Video::vcGlobals::write_frames_to_file)
        {
            size_t nbytes = VideoCapture::write_frame_to_file(logger, filestream, sp_frame);
            assert (nbytes == sp_frame->num_valid_elements());

            //////////////////////////////////////////////////////////////////////
            // Used in the code for DEBUG purposes only to simulate a heavy load.
            // Do not un-comment it lightly.
            // std::this_thread::sleep_for(std::chrono::milliseconds(40));
            //////////////////////////////////////////////////////////////////////
        }

        if (Video::vcGlobals::write_frames_to_process)
        {
            size_t nbytes = VideoCapture::write_frame_to_process(logger, processstream, sp_frame);
            assert (nbytes == sp_frame->num_valid_elements());

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
                              << Util::Utility::get_errno_message(errnocopy);
        }
    }
}
#endif // 0

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
    if (p != NULL)
    {
        size_t bytesleft = bsize;
        uint8_t *startdata = static_cast<uint8_t*>(p);
        size_t nextsize = 0;

        while (bytesleft > 0)
        {
            nextsize = bytesleft > EnetUtil::NtwkUtilBufferSize? EnetUtil::NtwkUtilBufferSize : bytesleft;

            std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp =
                        EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>::create(startdata, nextsize);

            startdata += nextsize;
            bytesleft -= nextsize;
            assert (sp->num_valid_elements() == nextsize);

            // Add the shared_ptr to the queue
            VideoCapture::video_capture_queue::s_ringbuf.put(sp, VideoCapture::video_capture_queue::s_condvar);
        }
    }
}

#if 0

// Open/truncate the output file that will hold captured frames
FILE * VideoCapture::create_output_file(Log::Logger logger)
{
    int errnocopy = 0;
    FILE *output_stream = NULL;

    if ((output_stream = ::fopen (Video::vcGlobals::output_file.c_str(), "w+")) == NULL)
    {
        errnocopy = errno;
        loggerp->error() << "Cannot create/truncate output file \"" <<
        Video::vcGlobals::output_file << "\": " << Util::Utility::get_errno_message(errnocopy);
    }
    else
    {
        loggerp->debug() << "Created/truncated output file \"" << Video::vcGlobals::output_file << "\"";
    }
    return output_stream;
}

// Start up the process that will receive video frames in it's std input
FILE * VideoCapture::create_output_process(Log::Logger logger)
{
    int errnocopy = 0;
    FILE *output_stream = NULL;

    // Need to refresh output process since command line options
    // may have changed the requested pixel-format.
    Json::Value& cfg_root = Config::ConfigSingleton::GetJsonRootCopyRef();

    std::string procIndicator;

    if (Video::vcGlobals::use_other_proc)
    {
        // Use the "other" entry in the "pixel-format" section
        procIndicator = "other";
    }
    else
    {
        // use the process string indicated by the "preferred-pixel-format" indicator
        procIndicator = (Video::vcGlobals::pixel_format == Video::pxl_formats::h264? "h264": "yuyv");
    }

    Video::vcGlobals::output_process = cfg_root["Config"]
                                                ["Video"]
                                                 ["frame-capture"]
                                                  [Video::vcGlobals::video_grabber_name]
                                                   ["pixel-format"]
                                                    [procIndicator]
                                                     ["output-process"].asString();
    std::string actual_process;
    if (Video::vcGlobals::proc_redir)
    {
        actual_process = Video::vcGlobals::output_process + " 2> " + Video::vcGlobals::redir_filename;
    }
    else
    {
        actual_process = Video::vcGlobals::output_process;
    }
    loggerp->debug() << "\nraw_buffer_queue_handler: Updated output process to:  " << actual_process;

    if ((output_stream = ::popen (actual_process.c_str(), "w")) == NULL)
    {
        errnocopy = errno;
        loggerp->error() << "Could not start the process \"" << actual_process
                       << "\": " << Util::Utility::get_errno_message(errnocopy);
    }
    else
    {
        loggerp->debug() << "Started the process \"" << actual_process << "\".";
    }
    return output_stream;
}

size_t VideoCapture::write_frame_to_file(Log::Logger logger, FILE *filestream,
            std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp_frame)
{
    size_t elementswritten = std::fwrite(sp_frame->data().data(), sizeof(uint8_t), sp_frame->num_valid_elements(), filestream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_valid_elements())
    {
        loggerp->error() << "VideoCapture::write_frame_to_file: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_valid_elements() << ", got " << byteswritten << " bytes: " <<
                        Util::Utility::get_errno_message(errnocopy);
    }
    fflush(filestream);

    return byteswritten;
}

size_t VideoCapture::write_frame_to_process(Log::Logger logger, FILE *processstream,
            std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp_frame)
{
    size_t elementswritten = std::fwrite(sp_frame->data().data(), sizeof(uint8_t), sp_frame->num_valid_elements(), processstream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_valid_elements())
    {
        loggerp->error() << "VideoCapture::write_frame_to_process: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_valid_elements() << ", got " << byteswritten << " bytes: " <<
                        Util::Utility::get_errno_message(errnocopy);
    }
    fflush(processstream);

    return byteswritten;
}

#endif // 0

