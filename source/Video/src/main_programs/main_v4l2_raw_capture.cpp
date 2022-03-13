//
// This is the C++ main fronting for the C main (called here v4l2_raw_capture_main()).
//

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

#include <video_capture_raw_queue.hpp>
#include <v4l2_raw_capture.h>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>

std::string logChannelName = "v4l2_raw_capture";
std::string logFilelName = logChannelName + "_log.txt";
Log::Log::Level loglevel = Log::Log::Level::eDebug;

/////////////////////////////////////////////////////////////////////////////
// Set up logging facility (for the C code) roughly equivalent to std::cerr...
/////////////////////////////////////////////////////////////////////////////

// Filthy code but I have to deal with C.
Log::Logger *global_logger = NULL;

#ifdef __cplusplus
    extern "C" void v4l2capture_logger(const char *logmessage);
#else
    void v4l2capture_logger(const char *logmessage);
#endif // __cplusplus

void v4l2capture_logger(const char *logmessage)
{
    if (global_logger == NULL)
    {
        fprintf (stderr, "%s\n", logmessage);
    }
    else
    {
        global_logger->debug() << logmessage;
    }
}

void (*logger_function)(const char *logmessage) = v4l2capture_logger;

/////////////////////////////////////////////////////////////////////////////
// Setup of the callback function (from the C code).
// Called back for every buffer that becomes available available.
/////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
    extern "C" void v4l2capture_callback(void *p, size_t size);
#else
    void v4l2capture_callback(void *p, size_t size);
#endif // __cplusplus

void v4l2capture_callback(void *p, size_t bsize)
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

void (*callback_function)(void *, size_t) = v4l2capture_callback;

int main(int argc, char *argv[])
{
    Log::Config::Vector configList;
    Util::Utility::initializeLogManager(configList, loglevel, logFilelName, Util::Utility::disableConsole, Util::Utility::enableLogFile);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName.c_str());

    // Setup of the logger callback function (from the C code).
    global_logger = &logger;

    std::thread queuethread;
    int ret = 0;
    try
    {
        // Start the thread which handles the queue of raw buffers that the callback
        // function handles.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler, logger);

        // This is the C based code close to the hardware
        ret = ::v4l2_raw_capture_main(argc, argv, callback_function, logger_function);

        // Inform the queue handler thread that the party is over...
        VideoCapture::video_capture_queue::set_terminated(true);

       // Make sure the ring buffer gets emptied
        VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    }
    catch (std::exception &exp)
    {
        logger.error()
                << "Got exception in main() starting the raw_buffer_queue_handler thread "
                << exp.what();
        ret = 1;
    } catch (...)
    {
        logger.error()
                << "General exception occurred in MAIN() the raw_buffer_queue_handler thread ";
        ret = 1;
    }

    // Wait for the queue thread to finish
    if (queuethread.joinable()) queuethread.join();

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return ret;
}

