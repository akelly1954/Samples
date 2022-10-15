
// COPYRIGHTS:
//
// Significant portions of this code were taken from the documentation provided by
// kernel.org in the "V4L2 API" section called "V4L2 video capture example" -
//
//          kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html
//
// - and modified (as of 3-10-2022). From the text provided there and from
// https://linuxtv.org/docs.php, the portions of code used can be used and distributed
// without restrictions.
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

// PLEASE NOTE:  The streaming method IO_METHOD_MMAP is the only one actually
// tested.  Please do not use IO_METHOD_USERPTR or IO_METHOD_READ until they are tested.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>             /* getopt_long() */

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <vidcap_v4l2_interface.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <linux/videodev2.h>



void VideoCapture::vidcap_v4l2_interface::initialize()
{
    ;
}


void VideoCapture::vidcap_v4l2_interface::run()
{
    int slp = 1000;  // millisecods

    logger.debug() << "V4L2 video capture thread started...";

    // Wait for main() to signal us to start
    vidcap_capture_base::s_condvar.wait_for_ready();

    logger.debug() << "V4L2 video capture thread kick-started...";

    for (size_t i = 1; i < 10; i++)
    {
        if (!vidcap_capture_base::s_terminated) break;

        logger.info() << "\n\n       ***** " << i << ": V4L2 VIDEO CAPTURE NOT IMPLEMENTED YET *****\n\n";

        std::this_thread::sleep_for(std::chrono::milliseconds(slp));
    }

    logger.debug() << "V4L2 video Capture thread terminating ...";
}

