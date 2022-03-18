

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
#include <v4l2_raw_capture.h>

void    (*v4l2capture_callback_function)(void *, size_t) = NULL;
void    (*v4l2capture_logger_function)(const char *) = NULL;
void    (*v4l2capture_logger_stream_function)(const char *) = NULL;

void v4l2capture_set_callback_function(void (*callback_function)(void *, size_t)) // can be NULL
{
    v4l2capture_callback_function = callback_function;
}

void v4l2capture_set_logger_function(void (*logger_function)(const char *)) // can be NULL
{
    v4l2capture_logger_function = logger_function;
}

void v4l2capture_set_logger_stream_function(void (*logger_stream_function)(const char *)) // can be NULL
{
    v4l2capture_logger_stream_function = logger_stream_function;
}

void v4l2capture_set_callback_functions(
                    void (*callback_function)(void *, size_t),
                    void (*logger_function)(const char *),
                    void (*logger_stream_function)(const char *)
                    )
{
    v4l2capture_callback_function = callback_function;
    v4l2capture_logger_function = logger_function;
    v4l2capture_logger_stream_function = logger_stream_function;
}

// Interface functions

void v4l2capture_errno_exit(const char *s)
{
    {
        char sbuf[256];
        snprintf(sbuf, sizeof(sbuf), "%s error %d, %s", s, errno, strerror(errno));
        LOGGER_STDERR(sbuf);
    }

    exit(EXIT_FAILURE);
}











// Use this struct like this:  string_methods.name[io] (see v4l2_raw_capture.c
// for enum io_method io)
struct string_io_methods string_methods = { "IO_METHOD_READ", "IO_METHOD_MMAP", "IO_METHOD_USERPTR" };


#ifdef NOBUILD
        {
            // All this printing is done at init time, so no big deal.
            char sbuf[256];
            snprintf(sbuf, sizeof(sbuf), "driver: frame: %lu x %lu", fmt.fmt.pix.width, fmt.fmt.pix.height);
            LOGGER_STDERR(sbuf);

            char bfp[256];
            switch(fmt.fmt.pix.pixelformat)
            {
            case V4L2_PIX_FMT_YUYV:
                strcpy (bfp, "YUYV: aka \"YUV 4:2:2\": Packed format with ½ horizontal chroma resolution");
                break;
            case V4L2_PIX_FMT_H264:
                strcpy (bfp, "H264: H264 with start codes");
                break;
            default:
                "See </usr/include/linux/videodev2.h> for all format encoding macros";
                break;
            }

            snprintf(sbuf, sizeof(sbuf), "driver: pixel format set to 0x%lX - %s", fmt.fmt.pix.pixelformat, bfp);
            LOGGER_STDERR(sbuf);
        }
#endif // NOBUILD
