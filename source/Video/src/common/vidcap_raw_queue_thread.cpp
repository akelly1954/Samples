#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_profiler_thread.hpp>
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

bool video_capture_queue::s_terminated = false;
bool video_capture_queue::s_write_frames_to_file = true;
size_t video_capture_queue::s_write_frame_count = 200;
Util::condition_data<int> video_capture_queue::s_condvar(0);
Util::circular_buffer<std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>>>
                                                                            video_capture_queue::s_ringbuf(100);

void VideoCapture::raw_buffer_queue_handler(Log::Logger logger, std::string output_file, bool profiling_enabled)
{
    FILE *filestream = NULL;

    // Captured frames also go to the output file only if the
    // option is set (which it is not, by default)
    if (video_capture_queue::s_write_frames_to_file)
    {

        filestream = create_output_file(logger, output_file);
        if (filestream == NULL)
        {
            // detailed error message already emitted by the create function
            logger.error() << "Exiting...";
            video_capture_queue::set_terminated(true);
            return;
        }
    }

    logger.debug() << "In VideoCapture::raw_buffer_queue_handler(): created " << output_file << ".";

    while (!video_capture_queue::s_terminated)
    {
        // logger.debug() << "In VideoCapture::raw_buffer_queue_handler(): not terminated, waiting on condvar 1";
        video_capture_queue::s_condvar.wait_for_ready();

        // logger.debug() << "In VideoCapture::raw_buffer_queue_handler(): kick started, looping";
        while (!video_capture_queue::s_terminated && !video_capture_queue::s_ringbuf.empty())
        {
            auto sp_frame = video_capture_queue::s_ringbuf.get();

            if (video_capture_queue::s_write_frames_to_file)
            {
                size_t nbytes = VideoCapture::write_frame_to_file(logger, filestream, output_file, sp_frame);
                assert (nbytes == sp_frame->num_valid_elements());

                //////////////////////////////////////////////////////////////////////
                // Used in the code for DEBUG purposes only to simulate a heavy load.
                // Do not un-comment it lightly.
                // std::this_thread::sleep_for(std::chrono::milliseconds(40));
                //////////////////////////////////////////////////////////////////////
            }
        }
    }

    logger.debug() << "Queue thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!video_capture_queue::s_ringbuf.empty())
    {
        auto sp_frame = video_capture_queue::s_ringbuf.get();
        logger.debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_valid_elements() << " bytes ";
        if (video_capture_queue::s_write_frames_to_file)
        {
            size_t nbytes = VideoCapture::write_frame_to_file(logger, filestream, output_file, sp_frame);
            assert (nbytes == sp_frame->num_valid_elements());

            //////////////////////////////////////////////////////////////////////
            // Used in the code for DEBUG purposes only to simulate a heavy load.
            // Do not un-comment it lightly.
            // std::this_thread::sleep_for(std::chrono::milliseconds(40));
            //////////////////////////////////////////////////////////////////////
        }
    }
    if (video_capture_queue::s_write_frames_to_file && filestream != NULL)
    {
        fflush(filestream);
        fclose(filestream);
    }
}

void video_capture_queue::set_terminated(bool t)
{
    video_capture_queue::s_terminated = t;
}

void video_capture_queue::set_write_frame_count(size_t count)
{
    video_capture_queue::s_write_frame_count = count;
}

void video_capture_queue::set_write_frames_to_file(bool t)
{
    video_capture_queue::s_write_frames_to_file = t;
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

// Open/truncate the output file that will hold captured frames
FILE * VideoCapture::create_output_file(Log::Logger logger, std::string output_file)
{
    int errnocopy = 0;
    FILE *output_stream = NULL;

    if ((output_stream = ::fopen (output_file.c_str(), "w+")) == NULL)
    {
        errnocopy = errno;
        logger.error() << "Cannot create/truncate output file \"" <<
        output_file << "\": " << Util::Utility::get_errno_message(errnocopy);
    }
    else
    {
        logger.debug() << "Created/truncated output file \"" << output_file << "\"";
    }
    return output_stream;
}

size_t VideoCapture::write_frame_to_file(Log::Logger logger, FILE *filestream, std::string output_file,
                          std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp_frame)
{
    size_t elementswritten = std::fwrite(sp_frame->data().data(), sizeof(uint8_t), sp_frame->num_valid_elements(), filestream);
    int errnocopy = errno;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_valid_elements())
    {
        logger.error() << "VideoCapture::write_frame_to_file: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_valid_elements() << ", got " << byteswritten << " bytes: " <<
                        Util::Utility::get_errno_message(errnocopy);
    }
    fflush(filestream);

    return byteswritten;
}


