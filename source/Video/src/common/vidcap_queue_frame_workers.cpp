
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

///////////////////////////////////////////////////////////////////////
// Member functions for the write-to-process class are in the
// second half of this file
///////////////////////////////////////////////////////////////////////


// The first part (here) includes the declarations of
// the write-to-file thread/queue
VideoCapture::write2file_frame_worker::write2file_frame_worker(size_t elements_in_ring_buffer)
            : frame_worker_thread_base (std::string("write_frames_to file"), elements_in_ring_buffer)
{
    setup();
    splogger->debug() << "write2file_frame_worker constructor: setup called.";
}

void VideoCapture::write2file_frame_worker::setup()
{
    video_capture_queue::register_worker(this);
    filestream = create_output_file();
    if (filestream == NULL)
    {
        // detailed error message already emitted by the create function
        splogger->error() << "Exiting...";
        set_terminated(true);
        return;
    }
    splogger->debug() << "In write2file_frame_worker::setup(): Successfully opened file \"" << Video::vcGlobals::output_file << "\".";
}

void VideoCapture::write2file_frame_worker::run()
{
    splogger->debug() << "write2file_frame_worker::run(): thread is running....";

    while (!m_terminated)
    {
        m_condvar.wait_for_ready();

        while (!m_terminated && !m_ringbuf.empty())
        {
            // This shared_ptr serves all consumers of this particular video data buffer
            auto sp_frame = m_ringbuf.get();

            size_t nbytes = write_frame_to_file(filestream, sp_frame);
            assert (nbytes == sp_frame->num_items());

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

void VideoCapture::write2file_frame_worker::finish()
{
    using Util::Utility;

    splogger->debug() << "write2file_frame_worker thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!m_ringbuf.empty())
    {
        auto sp_frame = m_ringbuf.get();
        splogger->debug() << "From queue (after terminate): Got buffer with " << sp_frame->num_items() << " bytes ";
        size_t nbytes = write_frame_to_file(filestream, sp_frame);
        assert (nbytes == sp_frame->num_items());

        //////////////////////////////////////////////////////////////////////
        // Used in the code for DEBUG purposes only to simulate a heavy load.
        // Do not un-comment it lightly.
        // std::this_thread::sleep_for(std::chrono::milliseconds(40));
        //////////////////////////////////////////////////////////////////////
     }

    fflush(filestream);
    fclose(filestream);
}

void VideoCapture::write2file_frame_worker::set_terminated(bool t)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

    m_terminated = t;
    video_capture_queue::set_terminated(t);

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    m_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    if (t)
    {
        splogger->debug() << "write2file_frame_worker: terminating...";
    }
    else
    {
        splogger->debug() << "write2file_frame_worker: termination set to FALSE...";
    }
}

void VideoCapture::write2file_frame_worker::add_buffer_to_queue(Util::shared_ptr_uint8_data_t sp)
{
    m_ringbuf.put(sp, m_condvar);
}

// Start up the process that will receive video frames in it's std input
FILE * VideoCapture::write2file_frame_worker::create_output_file()
{
    using Util::Utility;

    int errnocopy = 0;
    FILE *output_stream = NULL;

    if ((output_stream = ::fopen (Video::vcGlobals::output_file.c_str(), "w+")) == NULL)
    {
        errnocopy = errno;
        splogger->error() << "Cannot create/truncate output file \"" <<
        Video::vcGlobals::output_file << "\": " << Utility::get_errno_message(errnocopy);
    }
    else
    {
        splogger->debug() << "Created/truncated output file \"" << Video::vcGlobals::output_file << "\"";
    }
    return output_stream;
}

size_t VideoCapture::write2file_frame_worker
            ::write_frame_to_file(FILE *filestream, Util::shared_ptr_uint8_data_t sp_frame)
{
    using Util::Utility;

    size_t elementswritten = std::fwrite(sp_frame->_begin(), sizeof(uint8_t), sp_frame->num_items(), filestream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_items())
    {
        splogger->error() << "VideoCapture::write_frame_to_file: fwrite returned a short count or 0 bytes written. Requested: " <<
                             sp_frame->num_items() << ", got " << byteswritten << " bytes: " <<
                             Utility::get_errno_message(errnocopy);
    }
    fflush(filestream);
    return byteswritten;
}


///////////////////////////////////////////////////////////////////////
// The second part (here) includes the declarations of
// the write-to-process thread/queue
///////////////////////////////////////////////////////////////////////

VideoCapture::write2process_frame_worker::write2process_frame_worker(size_t elements_in_ring_buffer)
            : frame_worker_thread_base (std::string("write_frames_to process"), elements_in_ring_buffer)
{
    setup();
    splogger->debug() << "write2process_frame_worker constructor: setup called.";
}

void VideoCapture::write2process_frame_worker::setup()
{
    video_capture_queue::register_worker(this);
    processstream = create_output_process();
    if (processstream == NULL)
    {
        // detailed error message already emitted by the create function
        splogger->error() << "Exiting...";
        set_terminated(true);
        return;
    }
    splogger->debug() << "In write2process_frame_worker::setup(): Successfully started \"" << Video::vcGlobals::output_process << "\".";
}

void VideoCapture::write2process_frame_worker::run()
{
    splogger->debug() << "write2process_frame_worker::run(): thread is running....";

    while (!m_terminated)
    {
        m_condvar.wait_for_ready();

        while (!m_terminated && !m_ringbuf.empty())
        {
            // This shared_ptr serves all consumers of this particular video data buffer
            auto sp_frame = m_ringbuf.get();

            size_t nbytes = write_frame_to_process(processstream, sp_frame);
            assert (nbytes == sp_frame->num_items());

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

void VideoCapture::write2process_frame_worker::finish()
{
    using Util::Utility;

    splogger->debug() << "write2process_frame_worker thread terminating ...";

    // terminating: clear out the circular buffer queue
    while (!m_ringbuf.empty())
    {
        auto sp_frame = m_ringbuf.get();
        splogger->debug() << "write2process_frame_worker::finish(): From queue (after terminate): Got buffer with " << sp_frame->num_items() << " bytes ";

        size_t nbytes = write_frame_to_process(processstream, sp_frame);
        assert (nbytes == sp_frame->num_items());

        //////////////////////////////////////////////////////////////////////
        // Used in the code for DEBUG purposes only to simulate a heavy load.
        // Do not un-comment it lightly.
        // std::this_thread::sleep_for(std::chrono::milliseconds(40));
        //////////////////////////////////////////////////////////////////////
    }

    splogger->debug() << "Shutting down the process \""
                   << Video::vcGlobals::output_process << "\" (fflush, pclose()): ";
    fflush(processstream);
    int errnocopy = 0;
    if (::pclose(processstream) == -1)
    {
        errnocopy = errno;
        splogger->error() << "Error shutting down the process \""
                          << Video::vcGlobals::output_process << "\" on pclose(): "
                          << Utility::get_errno_message(errnocopy);
    }
}

void VideoCapture::write2process_frame_worker::set_terminated(bool t)
{
    std::lock_guard<std::mutex> lock(video_capture_queue::capture_queue_mutex);

    m_terminated = t;
    video_capture_queue::set_terminated(t);

    // Free up a potential wait on the condition variable
    // so that the thread can be terminated (otherwise it may hang).
    m_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    if (t)
    {
        splogger->debug() << "write2process_frame_worker: terminating...";
    }
    else
    {
        splogger->debug() << "write2process_frame_worker: termination set to FALSE...";
    }

}

void VideoCapture::write2process_frame_worker::add_buffer_to_queue(Util::shared_ptr_uint8_data_t sp)
{
    m_ringbuf.put(sp, m_condvar);
}

// Start up the process that will receive video frames in it's std input
FILE * VideoCapture::write2process_frame_worker::create_output_process()
{
    using Util::Utility;

    int errnocopy = 0;
    FILE *output_stream = NULL;

    auto ifptr = VideoCapture::video_plugin_base::interface_ptr;
    if (ifptr == nullptr)
    {
        throw std::runtime_error("create_output_process: Got a NULL interface pointer.");
    }

    ifptr->set_popen_process_string();
    std::string actual_process = video_plugin_base::popen_process_string;

    if (actual_process == "")
    {
        throw std::runtime_error("create_output_process: Got an empty process string.");
    }

    splogger->debug() << "create_output_process: Starting output process:  " << Utility::string_enquote(actual_process);

    if ((output_stream = ::popen (actual_process.c_str(), "w")) == NULL)
    {
        errnocopy = errno;
        splogger->error() << "create_output_process: Could not start the process " << Utility::string_enquote(actual_process)
                       << ": " << Utility::get_errno_message(errnocopy);
    }
    else
    {
        splogger->debug() << "create_output_process: Started the process " << Utility::string_enquote(actual_process) << ".";
    }
    return output_stream;
}

size_t VideoCapture::write2process_frame_worker
            ::write_frame_to_process(FILE *processstream, Util::shared_ptr_uint8_data_t sp_frame)
{
    using Util::Utility;

    size_t elementswritten = std::fwrite(sp_frame->_begin(), sizeof(uint8_t), sp_frame->num_items(), processstream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(uint8_t);

    if (byteswritten != sp_frame->num_items())
    {
        splogger->error() << "write_frame_to_process: fwrite returned a short count or 0 bytes written. Requested: " <<
                        sp_frame->num_items() << ", got " << byteswritten << " bytes: " <<
                        Utility::get_errno_message(errnocopy);
    }
    fflush(processstream);
    return byteswritten;
}

