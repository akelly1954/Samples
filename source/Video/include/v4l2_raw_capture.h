
/*
 * From kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html 3-10-2022
 */

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

#ifndef _V4L2_RAW_CAPTURE_H_
#define _V4L2_RAW_CAPTURE_H_

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

#include <linux/videodev2.h>

#ifdef __cplusplus
    extern "C" {
#endif

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
        IO_METHOD_READ,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};

static void errno_exit(const char *s);
static int xioctl(int fh, int request, void *arg);
static void process_image(const void *p, int size);
static int read_frame(void);
static void mainloop(void);
static void stop_capturing(void);
static void start_capturing(void);
static void uninit_device(void);
static void init_read(unsigned int buffer_size);
static void init_mmap(void);
static void init_userp(unsigned int buffer_size);
static void init_device(void);
static void close_device(void);
static void open_device(void);

#ifdef __cplusplus
extern "C" int v4l2_raw_capture_main(int argc, char *argv[]);
}
#else
int v4l2_raw_capture_main(int argc, char *argv[]);
#endif


#endif /*  _V4L2_RAW_CAPTURE_H_  */

