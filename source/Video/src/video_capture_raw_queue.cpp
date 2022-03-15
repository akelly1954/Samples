#include <video_capture_raw_queue.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
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
bool video_capture_queue::s_write_frames_to_file = false;
size_t video_capture_queue::s_write_frame_count = 200;       // gets me a bit more than 100Mb output.  YMMV.

Util::condition_data<int> video_capture_queue::s_condvar(0);
Util::circular_buffer<std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>>>
                                                                            video_capture_queue::s_ringbuf(100);

void VideoCapture::raw_buffer_queue_handler(Log::Logger logger, std::string output_file)
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

    while (!video_capture_queue::s_terminated)
    {
        video_capture_queue::s_condvar.wait_for_ready();

        while (!video_capture_queue::s_terminated && !video_capture_queue::s_ringbuf.empty())
        {
            auto sp_frame = video_capture_queue::s_ringbuf.get();
            logger.debug() << "From queue: Got buffer with " << sp_frame->num_valid_elements() << " bytes ";

            if (video_capture_queue::s_write_frames_to_file)
            {
                size_t nbytes = VideoCapture::write_frame_to_file(logger, filestream, output_file, sp_frame);
                assert (nbytes == sp_frame->num_valid_elements());
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






