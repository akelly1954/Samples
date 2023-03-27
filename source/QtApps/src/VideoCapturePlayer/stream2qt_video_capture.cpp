
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

#include <stream2qt_video_capture.hpp>
#include <nonqt_util.hpp>

///////////////////////////////////////////////////////////////////////
//
// Member functions for (mostly pure) virtual class stream2qt_video_capture
//
///////////////////////////////////////////////////////////////////////

VideoCapture::stream2qt_video_capture::stream2qt_video_capture(size_t elements_in_ring_buffer)
            : frame_worker_thread_base (std::string("stream2qt_video_capture"), elements_in_ring_buffer)
{
    ;
}

void VideoCapture::stream2qt_video_capture::setup()
{
    video_capture_queue::register_worker(this);
}

void VideoCapture::stream2qt_video_capture::run()
{
    using NonQtUtil::nqUtil;

    splogger->debug() << "stream2qt_video_capture::run(): thread is running....";

    if (!initialized)
    {
        setup();
        initialized = true;
        splogger->debug() << "sstream2qt_video_capture::run(): setup completed.";
    }

    while (!m_terminated)
    {
        m_condvar.wait_for_ready();
        static int count = 0;

        // Allow the ring buffer to fill up until the main window
        // is ready.
        for (count = 0; nqUtil::mwp == nullptr; count++)
        {
            if (count > 10)
            {
                splogger->debug() << "stream2qt_video_capture::run(): FATAL ERROR: MainWindow is not ready. Terminating...";
                std::cerr << "stream2qt_video_capture::run(): FATAL ERROR: MainWindow is not ready. Terminating..." << std::endl;
                set_terminated(true);
                return;
            }
            else if ((count % 5) == 0)
            {
                splogger->debug() << "stream2qt_video_capture::run(): (count=" << count << ") Waiting for MainWindow to be established.";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (nqUtil::mwp->getPlayer() == nullptr)
        {
            splogger->debug() << "stream2qt_video_capture::run(): FATAL ERROR: video player in NULL. Terminating...";
            std::cerr << "stream2qt_video_capture::run(): FATAL ERROR: video player in NULL. Terminating..." << std::endl;
            set_terminated(true);
            return;
        }

        while (!m_terminated && !m_ringbuf.empty())
        {
            // This shared_ptr serves all consumers of this particular video data buffer
            auto sp_frame = m_ringbuf.get();
            nqUtil::mwp->getPlayer()->receiveFrameBuffer(sp_frame);

            //////////////////////////////////////////////////////////////////////
            // Used in the code for DEBUG purposes only to simulate a heavy load.
            // Do not un-comment it lightly.
            // std::this_thread::sleep_for(std::chrono::milliseconds(40));
            //////////////////////////////////////////////////////////////////////
        }
    }
    finish();
}

// With m_terminated true, flush out the ring buffer
// and terminate the thread (return)

void VideoCapture::stream2qt_video_capture::finish()
{
    using Util::Utility;

    splogger->debug() << "stream2qt_video_capture thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!m_ringbuf.empty())
    {
        auto sp_frame = m_ringbuf.get();
        splogger->debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_items() << " bytes ";
        // size_t nbytes = write_frame_to_file(sp_frame);
        // assert (nbytes == sp_frame->num_items());

        //////////////////////////////////////////////////////////////////////
        // Used in the code for DEBUG purposes only to simulate a heavy load.
        // Do not un-comment it lightly.
        // std::this_thread::sleep_for(std::chrono::milliseconds(40));
        //////////////////////////////////////////////////////////////////////
     }
}

void VideoCapture::stream2qt_video_capture::set_terminated(bool t)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

    m_terminated = t;
    video_capture_queue::set_terminated(t);

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    m_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    if (t)
    {
        splogger->debug() << "stream2qt_video_capture: terminating...";
    }
    else
    {
        splogger->debug() << "stream2qt_video_capture: termination set to FALSE...";
    }
}

void VideoCapture::stream2qt_video_capture::add_buffer_to_queue(Util::shared_ptr_uint8_data_t sp)
{
    m_ringbuf.put(sp, m_condvar);
}
