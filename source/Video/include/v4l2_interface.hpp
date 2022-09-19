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
#include <LoggerCpp/LoggerCpp.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

extern Log::Logger& global_logger;
extern Log::Log::Level loglevel;
extern std::string log_level;

extern std::string logFilelName;
extern std::string logChannelName;

extern size_t framecount;
extern std::string frame_count;

extern bool profiling_enabled;
extern bool capture_finished;
extern bool capture_pause;
extern std::string output_file;

// C++ linkage for callback functions
extern bool v4l2capture_pause(void);
extern bool v4l2capture_finished(void);
extern void v4l2capture_terminate(int code, const char *logmessage);
extern void v4l2capture_logger(const char *logmessage);
extern void v4l2capture_stream_logger(const char *logmessage);
extern void v4l2capture_callback(void *p, size_t size);
extern void set_v4l2capture_finished(void);
extern void set_v4l2capture_pause(bool pause);

extern bool (*pause_function)();
extern bool (*finished_function)();
extern void (*terminate_function)(int code, const char *logmessage);
extern void (*logger_function)(const char *logmessage);
extern void (*logger_stream_function)(const char *logmessage);
extern void (*callback_function)(void *, size_t);

