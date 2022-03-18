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
#include <linux/videodev2.h>
#include <cppglue.h>

#ifdef __cplusplus
    extern "C" {
#endif

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

#ifdef __cplusplus
} // extern "C"

// Interface functions
extern "C" void v4l2capture_errno_exit(const char *s);
extern "C" int v4l2capture_xioctl(int fh, int request, void *arg);
extern "C" void v4l2capture_process_image(void *p, int size);
extern "C" int v4l2capture_read_frame(void);
extern "C" void v4l2capture_mainloop(void);
extern "C" void v4l2capture_stop_capturing(void);
extern "C" void v4l2capture_start_capturing(void);
extern "C" void v4l2capture_uninit_device(void);
extern "C" void v4l2capture_init_read(unsigned int buffer_size);
extern "C" void v4l2capture_init_mmap(void);
extern "C" void v4l2capture_init_userp(unsigned int buffer_size);
extern "C" void v4l2capture_init_device(void);
extern "C" void v4l2capture_close_device(void);
extern "C" void v4l2capture_open_device(void);
extern "C" int v4l2_raw_capture_main(int argc, char *argv[]);

#else // __cplusplus

// Interface functions
void v4l2capture_errno_exit(const char *s);
int v4l2capture_xioctl(int fh, int request, void *arg);
void v4l2capture_process_image(void *p, int size);
int v4l2capture_read_frame(void);
void v4l2capture_mainloop(void);
void v4l2capture_stop_capturing(void);
void v4l2capture_start_capturing(void);
void v4l2capture_uninit_device(void);
void v4l2capture_init_read(unsigned int buffer_size);
void v4l2capture_init_mmap(void);
void v4l2capture_init_userp(unsigned int buffer_size);
void v4l2capture_init_device(void);
void v4l2capture_close_device(void);
void v4l2capture_open_device(void);
int v4l2_raw_capture_main(int argc, char *argv[]);

#endif // __cplusplus

#endif /*  _V4L2_RAW_CAPTURE_H_  */

