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
#define LOGGER(x) if (v4l2capture_logger_function != NULL) { v4l2capture_logger_function(x); } else { ; }
#define LOGGER_STDERR(x) if (v4l2capture_logger_stream_function != NULL) { v4l2capture_logger_stream_function(x); } else { ; }

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

#ifdef __cplusplus
extern "C" {
#endif

    struct string_io_methods
    {
        const char *name[3];  // indexed by enum io_method
    };

    extern struct string_io_methods string_methods;
    extern bool (*v4l2capture_pause_function)();
    extern bool (*v4l2capture_finished_function)();
    extern void (*v4l2capture_callback_function)(void *, size_t);
    extern void (*v4l2capture_logger_function)(const char *);
    extern void (*v4l2capture_logger_stream_function)(const char *);

#ifdef __cplusplus
}

// This section is for CPP only, outside of the extern "C" block above.
extern "C" void v4l2capture_errno_exit(const char *s);
extern "C" void v4l2capture_exit(const char *s);
extern "C" void v4l2capture_exit_code(int code, const char *s);

// These functions serve as the "glue" between the C++ main and these C functions

extern "C" void v4l2capture_set_callback_functions(
                    bool (*pause_function)(),
                    bool (*finished_function)(),
                    void (*terminate_function)(int, const char *),
                    void (*callback_function)(void *, size_t),
                    void (*logger_function)(const char *),
                    void (*logger_stream_function)(const char *)
                   );
#else  // __cplusplus

// This section is for C only, no CPP
void v4l2capture_errno_exit(const char *s);
void v4l2capture_exit(const char *s);
void v4l2capture_exit_code(int code, const char *s);

// These functions serve as the "glue" between the C++ main and these C functions

void v4l2capture_set_callback_functions(
                    bool (*pause_function)(),
                    bool (*finished_function)(),
                    void (*terminate_function)(int, const char *),
                    void (*callback_function)(void *, size_t),
                    void (*logger_function)(const char *),
                    void (*logger_stream_function)(const char *)
                    );
#endif

