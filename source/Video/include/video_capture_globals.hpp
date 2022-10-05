#pragma once

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

#include <Utility.hpp>
#include <MainLogger.hpp>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

namespace Video
{
    struct vcGlobals
    {
        static std::string logChannelName;
        static std::string logFilelName;
        static std::string output_file;
        static Log::Log::Level loglevel;
        static std::string default_log_level;
        static std::string log_level;
        static size_t framecount;
        static std::string default_str_frame_count;
        static std::string str_frame_count;
        static bool profiling_enabled;
        static bool capture_finished;
        static bool capture_pause;
    };

} // end of namespace Video

// TODO: get rid of no-build ifdefs
#ifdef SKIP_BUILD

extern Log::Logger *global_logger;
extern Log::Log::Level loglevel;
extern std::string log_level;

extern std::string logFilelName;
extern std::string logChannelName;

extern size_t framecount;
extern std::string str_frame_count;

extern bool profiling_enabled;
extern bool capture_finished;
extern bool capture_pause;
extern std::string output_file;

// C++ linkage for callback functions
bool v4l2capture_pause(void);
bool v4l2capture_finished(void);
void v4l2capture_terminate(int code, const char *logmessage);
void v4l2capture_logger(const char *logmessage);
void v4l2capture_stream_logger(const char *logmessage);
void v4l2capture_callback(void *p, size_t size);
void set_v4l2capture_finished(void);
void set_v4l2capture_pause(bool pause);

struct string_io_methods
{
    const char *name[3];  // indexed by enum io_method
};

extern struct string_io_methods string_methods;
void v4l2capture_errno_exit(const char *s, int errnocopy);
void v4l2capture_exit(const char *s);
void v4l2capture_exit_code(int code, const char *s);

/*
 * Use the LOGGER(x) or LOGGER_STDERR(x) macro like this:
 *  {
 *         char sbuf[128];
 *         snprintf(sbuf, sizeof(sbuf), "Got frame with %ld bytes", buf.bytesused);
 *         LOGGER(sbuf);  /* OR... LOGGER_STDERR(sbuf);
 *  }
 *
 *  Don't add newlines at the end of the string.  The logger does that for you.
 *  OR:  Use the LOGGER_3Arg... etc for a shortcut
 */
#define LOGGER(x)             v4l2capture_logger(x)
#define LOGGER_STDERR(x)     v4l2capture_stream_logger(x)

#define LOGGER_1Arg(fmtstr, arg)   {                \
        char sbuf[512];                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg);  \
        LOGGER(sbuf);                               \
    }

#define LOGGER_2Arg(fmtstr, arg1, arg2)   {                 \
        char sbuf[512];                                     \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2);   \
        LOGGER(sbuf);                                       \
    }

#define LOGGER_3Arg(fmtstr, arg1, arg2, arg3)   {                   \
        char sbuf[512];                                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2, arg3);     \
        LOGGER(sbuf);                                               \
    }

#define LOGGER_STDERR_1Arg(fmtstr, arg)   {         \
        char sbuf[512];                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg);  \
        LOGGER_STDERR(sbuf);                               \
    }

#define LOGGER_STDERR_2Arg(fmtstr, arg1, arg2)   {          \
        char sbuf[512];                                     \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2);   \
        LOGGER_STDERR(sbuf);                                       \
    }

#define LOGGER_STDERR_3Arg(fmtstr, arg1, arg2, arg3)   {            \
        char sbuf[512];                                             \
        snprintf(sbuf, sizeof(sbuf), fmtstr, arg1, arg2, arg3);     \
        LOGGER_STDERR(sbuf);                                               \
    }

#endif // SKIP_BUILD


