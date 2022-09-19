#ifndef _V4L2_RAW_CAPTURE_H_
#define _V4L2_RAW_CAPTURE_H_

/*
 * MODIFIED FROM:
 *
 *       kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html (3-10-2022)
 */

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

#include <string.h>
#include <stdbool.h>
#include <linux/videodev2.h>
#include <cppglue.hpp>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

enum io_method {
        IO_METHOD_READ = 0,
        IO_METHOD_MMAP,
        IO_METHOD_USERPTR,
};

struct buffer {
        void   *start;
        size_t  length;
};


// Interface functions
extern int v4l2capture_xioctl(int fh, int request, void *arg);
extern void v4l2capture_process_image(void *p, int size);
extern int v4l2capture_read_frame(void);
extern void v4l2capture_mainloop(void);
extern void v4l2capture_stop_capturing(void);
extern void v4l2capture_start_capturing(void);
extern void v4l2capture_uninit_device(void);
extern void v4l2capture_init_read(unsigned int buffer_size);
extern void v4l2capture_init_mmap(void);
extern void v4l2capture_init_userp(unsigned int buffer_size);
extern void v4l2capture_init_device(void);
extern void v4l2capture_close_device(void);
extern void v4l2capture_open_device(void);
extern int v4l2_raw_capture_main(int argc, char *argv[]);

#endif /*  _V4L2_RAW_CAPTURE_H_  */

