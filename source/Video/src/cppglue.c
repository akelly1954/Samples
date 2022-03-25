

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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <v4l2_raw_capture.h>

bool    (*v4l2capture_pause_function)() = NULL;
bool    (*v4l2capture_finished_function)() = NULL;
void    (*v4l2capture_terminate_function)(int, const char *) = NULL;
void    (*v4l2capture_callback_function)(void *, size_t) = NULL;
void    (*v4l2capture_logger_function)(const char *) = NULL;
void    (*v4l2capture_logger_stream_function)(const char *) = NULL;

void v4l2capture_set_callback_functions(
                    bool (*pause_function)(),
                    bool (*finished_function)(),
                    void (*terminate_function)(int, const char *),
                    void (*callback_function)(void *, size_t),
                    void (*logger_function)(const char *),
                    void (*logger_stream_function)(const char *)
                    )
{
    v4l2capture_pause_function = pause_function;
    v4l2capture_finished_function = finished_function;
    v4l2capture_terminate_function = terminate_function;
    v4l2capture_callback_function = callback_function;
    v4l2capture_logger_function = logger_function;
    v4l2capture_logger_stream_function = logger_stream_function;
}

// Interface functions

void v4l2capture_errno_exit(const char *s)
{
    LOGGER_STDERR_3Arg("%s error %d, %s", s, errno, strerror(errno));
    v4l2capture_exit("error");
}

void v4l2capture_exit_code(int code, const char *s)
{
    if (v4l2capture_terminate_function) {
        (*v4l2capture_terminate_function)(code, s);
    }
    abort();
}

void v4l2capture_exit(const char *s)
{
    if (v4l2capture_terminate_function) {
        (*v4l2capture_terminate_function)(1, s);
    }
    abort();
}

// Use this struct like this:  string_methods.name[io] (see v4l2_raw_capture.c
// for enum io_method io)
struct string_io_methods string_methods = { "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };

