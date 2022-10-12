

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

#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <MainLogger.hpp>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

// Video::vcGlobals statics' definition

std::string     Video::vcGlobals::logChannelName =     "video_capture";
std::string     Video::vcGlobals::logFilelName =       Video::vcGlobals::logChannelName + "_log.txt";
std::string     Video::vcGlobals::output_file =        Video::vcGlobals::logChannelName + ".data";  // Name of file intended for the video frames
Log::Log::Level Video::vcGlobals::loglevel =           Log::Log::Level::eNotice;
std::string     Video::vcGlobals::default_log_level =  Log::Log::toString(Video::vcGlobals::loglevel);
std::string     Video::vcGlobals::log_level =          Video::vcGlobals::default_log_level;
size_t          Video::vcGlobals::framecount =         200;
std::string     Video::vcGlobals::default_str_frame_count(std::to_string(framecount));
std::string     Video::vcGlobals::str_frame_count(default_str_frame_count);
bool            Video::vcGlobals::profiling_enabled =  false;
bool            Video::vcGlobals::capture_finished =   false;
bool            Video::vcGlobals::capture_pause =      false;
std::string     Video::vcGlobals::config_file_name =   Video::vcGlobals::logChannelName + ".json";

// Video configuration
std::string     Video::vcGlobals::video_grabber_name =  "v4l2";


// This function overwrites values in Video::vcGlobals with content from
// the json config file.
//
// We do not use std::endl here to avoid flushing subtleties at the end of each line.
// We count on JsonCpp to throw an exception if the [] access to the node yields an error.
// Caller should try/catch(std::exception)'s.
bool Video::updateInternalConfigsWithJsonValues(std::ostream& strm, const Json::Value& cfg_root)
{
    using namespace Util;

    // logChannelName
    strm << "\nGetting config values from config file " << Video::vcGlobals::config_file_name << "\n";

    Video::vcGlobals::logChannelName = Utility::trim(cfg_root["Config"]["Logger"]["channel-name"].asString());
    strm << "\nFrom JSON:  Set logger channel-name to: " << Video::vcGlobals::logChannelName;

    // logFilelName
    Video::vcGlobals::logFilelName = Utility::trim(cfg_root["Config"]["Logger"]["file-name"].asString());
    strm << "\nFrom JSON:  Set logger file-name to: " << Video::vcGlobals::logFilelName;

    // loglevel and log_level
    Video::vcGlobals::log_level = Utility::trim(cfg_root["Config"]["Logger"]["log-level"].asString());
    strm << "\nFrom JSON:  Set default logger log level to: " << Video::vcGlobals::log_level;

    // video frame grabber
    Video::vcGlobals::video_grabber_name = Utility::trim(cfg_root["Config"]["Video"]["preferred-interface"].asString());
    strm << "\nFrom JSON:  Set default video-frame-grabber to: " << Video::vcGlobals::video_grabber_name;

    return true;
}


// TODO: get rid of no-build ifdefs

#ifdef SKIP_BUILD


//////////////////////////////////////////////////////////////////////////////////
// Static objects - these are staying
//////////////////////////////////////////////////////////////////////////////////






//////////////////////////////////////////////////////////////////////////////////
// Static objects (with some are also extern's) - see the .hpp file
//////////////////////////////////////////////////////////////////////////////////
std::string logChannelName = "v4l2_raw_capture";
std::string logFilelName = logChannelName + "_log.txt";
std::string output_file = logChannelName + ".data";  // Name of file intended for the video frames
Log::Log::Level loglevel = Log::Log::Level::eDebug;
std::string default_log_level = Log::Log::toString(loglevel);
std::string log_level = default_log_level;
size_t framecount = 200;
std::string default_str_frame_count(std::to_string(framecount));
std::string str_frame_count(default_str_frame_count);
bool profiling_enabled = false;
bool capture_finished = false;
bool capture_pause = false;

//////////////////////////////////////////////////////////////////////////////////
// Interface to C language functions section
//////////////////////////////////////////////////////////////////////////////////

// Set up logging facility (for the video code) roughly equivalent to std::cerr...
Log::Logger *global_logger = NULL;

bool v4l2capture_finished(void)
{
    return capture_finished;
}

// finished can only be set to true.
void set_v4l2capture_finished(void)
{
    capture_finished = true;
}

bool v4l2capture_pause(void)
{
    return capture_pause;
}

// pause is true when capturing is paused. Set to false to resume.
void set_v4l2capture_pause(bool pause)
{
    capture_pause = pause;
}

//////////////////////////////////////////////////////
// Exit the process (without hanging)
//////////////////////////////////////////////////////


void v4l2capture_terminate(int code, const char *logmessage)
{
    if (global_logger)
    {
        if (code == 0)
        {
            global_logger->info() << "Terminate process: exit code=" << code << ": " << logmessage;
        }
        else
        {
            global_logger->error() << "Terminate process: exit code=" << code << ": " << logmessage;
        }
    }

    fprintf (stderr, "Terminate process: exit code=%d: %s\n", code, logmessage);

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    ::_exit(code);     // See man page for _exit(2)
}

// Interface functions

void v4l2capture_errno_exit(const char *s, int errnocopy)
{
    std::string msg = std::string(s) + " error, errno=" + std::to_string(errnocopy) + ": "
            + const_cast<const char *>(strerror(errnocopy));
    LOGGER_STDERR(msg.c_str());
    v4l2capture_exit(msg.c_str());
}

void v4l2capture_exit_code(int code, const char *s)
{
    v4l2capture_terminate(code, s);
    abort();
}

void v4l2capture_exit(const char *s)
{
       v4l2capture_terminate(1, s);
    abort();
}

// Use this struct like this:  string_methods.name[io] (see v4l2_raw_capture.c
// for enum io_method io)
struct string_io_methods string_methods = { "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };


//////////////////////////////////////////////////////
// Log to log file if possible
//////////////////////////////////////////////////////

void v4l2capture_logger(const char *logmessage)
{
    if (global_logger) global_logger->info() << logmessage;
    std::cerr << logmessage << std::endl;
}

//////////////////////////////////////////////////////
// Log to screen
//////////////////////////////////////////////////////
void v4l2capture_stream_logger(const char *logmessage)
{
    std::cerr << logmessage << std::endl;
}

// Setup of the callback function (from the C code).
// Called back for every buffer that becomes available.
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

#endif // SKIP_BUILD
