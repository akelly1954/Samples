/*
 * Adapted from kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html 3-10-2022
 */

/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

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
#include <v4l2_raw_capture.h>

static char            *dev_name = NULL;
static enum io_method   io = IO_METHOD_MMAP;
static int              fd = -1;
struct buffer          *buffers = NULL;
static unsigned int     n_buffers = 0;
static int              out_buf = 1;   // This is now the default ("-o" flag).
static int              force_format = 0;
static int              frame_count = 70;
static void             (*v4l2capture_callback_function)(void *, size_t) = NULL;
static void             (*v4l2capture_logger_function)(const char *) = NULL;

void v4l2capture_set_callback_function(void (*callback_function)(void *, size_t)) // can be NULL
{
    if (callback_function != NULL)  v4l2capture_callback_function = callback_function;
}

/*
 * Use the LOGGER(x) macro like this:
 *  {
 *         char sbuf[128];
 *         snprintf(sbuf, sizeof(sbuf), "Got frame with %ld bytes", buf.bytesused);
 *         LOGGER(sbuf);
 *  }
 *
 *  Don't add newlines at the end of the string.  The logger does that for you.
 */
#define LOGGER(x) if (v4l2capture_logger_function != NULL) { v4l2capture_logger_function(x); } else { ; }

void v4l2capture_set_logger_function(void (*logger_function)(const char *)) // can be NULL
{
    if (logger_function != NULL)  v4l2capture_logger_function = logger_function;
}

void v4l2capture_errno_exit(const char *s)
{
    {
        char sbuf[256];
        snprintf(sbuf, sizeof(sbuf), "%s error %d, %s", s, errno, strerror(errno));
        LOGGER(sbuf);
    }

    exit(EXIT_FAILURE);
}

int v4l2capture_xioctl(int fh, int request, void *arg)
{
        int r;

        do {
                r = ioctl(fh, request, arg);
        } while (-1 == r && EINTR == errno);

        return r;
}

void v4l2capture_process_image(void *p, int size)
{
    // repurpose the -o flag to use callback instead of fwrite
    if (out_buf)
    {
        if (v4l2capture_callback_function != NULL) v4l2capture_callback_function(p, size);
    }
}

int v4l2capture_read_frame(void)
{
        struct v4l2_buffer buf;
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                v4l2capture_errno_exit("read");
                        }
                }

                v4l2capture_process_image(buffers[0].start, buffers[0].length);
                break;

        case IO_METHOD_MMAP:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_MMAP;

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                v4l2capture_errno_exit("VIDIOC_DQBUF");
                        }
                }

                assert(buf.index < n_buffers);

                v4l2capture_process_image(buffers[buf.index].start, buf.bytesused);

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_QBUF, &buf))
                        v4l2capture_errno_exit("VIDIOC_QBUF");
                break;

        case IO_METHOD_USERPTR:
                CLEAR(buf);

                buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory = V4L2_MEMORY_USERPTR;

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_DQBUF, &buf)) {
                        switch (errno) {
                        case EAGAIN:
                                return 0;

                        case EIO:
                                /* Could ignore EIO, see spec. */

                                /* fall through */

                        default:
                                v4l2capture_errno_exit("VIDIOC_DQBUF");
                        }
                }

                for (i = 0; i < n_buffers; ++i)
                        if (buf.m.userptr == (unsigned long)buffers[i].start
                            && buf.length == buffers[i].length)
                                break;

                assert(i < n_buffers);

                v4l2capture_process_image((void *)buf.m.userptr, buf.bytesused);

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_QBUF, &buf))
                        v4l2capture_errno_exit("VIDIOC_QBUF");
                break;
        }

        return 1;
}

void v4l2capture_mainloop(void)
{
        unsigned int count;

        count = frame_count;

        while (count-- > 0) {
                for (;;) {
                        fd_set fds;
                        struct timeval tv;
                        int r;

                        FD_ZERO(&fds);
                        FD_SET(fd, &fds);

                        /* Timeout. */
                        tv.tv_sec = 2;
                        tv.tv_usec = 0;

                        r = select(fd + 1, &fds, NULL, NULL, &tv);

                        if (-1 == r) {
                                if (EINTR == errno)
                                        continue;
                                v4l2capture_errno_exit("select");
                        }

                        if (0 == r) {
                                LOGGER("select timeout")
                                exit(EXIT_FAILURE);
                        }

                        if (v4l2capture_read_frame())
                                break;
                        /* EAGAIN - continue select loop. */
                }
        }
}

void v4l2capture_stop_capturing(void)
{
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2capture_xioctl(fd, VIDIOC_STREAMOFF, &type))
                        v4l2capture_errno_exit("VIDIOC_STREAMOFF");
                break;
        }
}

void v4l2capture_start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

        switch (io) {
        case IO_METHOD_READ:
                /* Nothing to do. */
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_MMAP;
                        buf.index = i;

                        if (-1 == v4l2capture_xioctl(fd, VIDIOC_QBUF, &buf))
                                v4l2capture_errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2capture_xioctl(fd, VIDIOC_STREAMON, &type))
                        v4l2capture_errno_exit("VIDIOC_STREAMON");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i) {
                        struct v4l2_buffer buf;

                        CLEAR(buf);
                        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                        buf.memory = V4L2_MEMORY_USERPTR;
                        buf.index = i;
                        buf.m.userptr = (unsigned long)buffers[i].start;
                        buf.length = buffers[i].length;

                        if (-1 == v4l2capture_xioctl(fd, VIDIOC_QBUF, &buf))
                                v4l2capture_errno_exit("VIDIOC_QBUF");
                }
                type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                if (-1 == v4l2capture_xioctl(fd, VIDIOC_STREAMON, &type))
                        v4l2capture_errno_exit("VIDIOC_STREAMON");
                break;
        }
}

void v4l2capture_uninit_device(void)
{
        unsigned int i;

        switch (io) {
        case IO_METHOD_READ:
                free(buffers[0].start);
                break;

        case IO_METHOD_MMAP:
                for (i = 0; i < n_buffers; ++i)
                        if (-1 == munmap(buffers[i].start, buffers[i].length))
                                v4l2capture_errno_exit("munmap");
                break;

        case IO_METHOD_USERPTR:
                for (i = 0; i < n_buffers; ++i)
                        free(buffers[i].start);
                break;
        }

        free(buffers);
}

void v4l2capture_init_read(unsigned int buffer_size)
{
        buffers = calloc(1, sizeof(*buffers));

        if (!buffers) {
                LOGGER("Out of memory");
                exit(EXIT_FAILURE);
        }

        buffers[0].length = buffer_size;
        buffers[0].start = malloc(buffer_size);

        if (!buffers[0].start) {
                LOGGER("Out of memory");
                exit(EXIT_FAILURE);
        }
}

void v4l2capture_init_mmap(void)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count = 4;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;

        if (-1 == v4l2capture_xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                {
                    char sbuf[256];
                    snprintf(sbuf, sizeof(sbuf), "%s does not support memory mapping", dev_name);
                    LOGGER(sbuf);
                }
                exit(EXIT_FAILURE);
            } else {
                v4l2capture_errno_exit("VIDIOC_REQBUFS");
            }
        }

        if (req.count < 2) {
            {
                char sbuf[256];
                snprintf(sbuf, sizeof(sbuf), "Insufficient buffer memory on %s", dev_name);
                LOGGER(sbuf);
            }
            exit(EXIT_FAILURE);
        }

        buffers = calloc(req.count, sizeof(*buffers));

        if (!buffers) {
            LOGGER("Out of memory");
            exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
                struct v4l2_buffer buf;

                CLEAR(buf);

                buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                buf.memory      = V4L2_MEMORY_MMAP;
                buf.index       = n_buffers;

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_QUERYBUF, &buf))
                        v4l2capture_errno_exit("VIDIOC_QUERYBUF");

                buffers[n_buffers].length = buf.length;
                buffers[n_buffers].start =
                        mmap(NULL /* start anywhere */,
                              buf.length,
                              PROT_READ | PROT_WRITE /* required */,
                              MAP_SHARED /* recommended */,
                              fd, buf.m.offset);

                if (MAP_FAILED == buffers[n_buffers].start)
                        v4l2capture_errno_exit("mmap");
        }
}

void v4l2capture_init_userp(unsigned int buffer_size)
{
        struct v4l2_requestbuffers req;

        CLEAR(req);

        req.count  = 4;
        req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_USERPTR;

        if (-1 == v4l2capture_xioctl(fd, VIDIOC_REQBUFS, &req)) {
            if (EINVAL == errno) {
                {
                    char sbuf[256];
                    snprintf(sbuf, sizeof(sbuf), "%s does not support user pointer i/o", dev_name);
                    LOGGER(sbuf);
                }
                exit(EXIT_FAILURE);
            } else {
                v4l2capture_errno_exit("VIDIOC_REQBUFS");
            }
        }

        buffers = calloc(4, sizeof(*buffers));

        if (!buffers) {
                LOGGER("Out of memory");
                exit(EXIT_FAILURE);
        }

        for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
                buffers[n_buffers].length = buffer_size;
                buffers[n_buffers].start = malloc(buffer_size);

                if (!buffers[n_buffers].start) {
                        LOGGER("Out of memory");
                        exit(EXIT_FAILURE);
                }
        }
}

void v4l2capture_init_device(void)
{
        struct v4l2_capability cap;
        struct v4l2_cropcap cropcap;
        struct v4l2_crop crop;
        struct v4l2_format fmt;
        unsigned int min;

        if (-1 == v4l2capture_xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
            if (EINVAL == errno) {
                {
                    char sbuf[256];
                    snprintf(sbuf, sizeof(sbuf), "%s is no V4L2 device", dev_name);
                    LOGGER(sbuf);
                }
                exit(EXIT_FAILURE);
            } else {
                v4l2capture_errno_exit("VIDIOC_QUERYCAP");
            }
        }

        if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
            {
                char sbuf[256];
                snprintf(sbuf, sizeof(sbuf), "%s is no capture device", dev_name);
                LOGGER(sbuf);
            }
            exit(EXIT_FAILURE);
        }

        switch (io) {
        case IO_METHOD_READ:
                if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
                    {
                        char sbuf[256];
                        snprintf(sbuf, sizeof(sbuf), "%s does not support read i/o", dev_name);
                        LOGGER(sbuf);
                    }
                    exit(EXIT_FAILURE);
                }
                break;

        case IO_METHOD_MMAP:
        case IO_METHOD_USERPTR:
                if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
                    {
                        char sbuf[256];
                        snprintf(sbuf, sizeof(sbuf), "%s does not support streaming i/o", dev_name);
                        LOGGER(sbuf);
                    }
                    exit(EXIT_FAILURE);
                }
                break;
        }


        /* Select video input, video standard and tune here. */


        CLEAR(cropcap);

        cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        if (0 == v4l2capture_xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
                crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                crop.c = cropcap.defrect; /* reset to default */

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_S_CROP, &crop)) {
                        switch (errno) {
                        case EINVAL:
                                /* Cropping not supported. */
                                break;
                        default:
                                /* Errors ignored. */
                                break;
                        }
                }
        } else {
                /* Errors ignored. */
        }


        CLEAR(fmt);

        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (force_format) {
                fmt.fmt.pix.width       = 640;
                fmt.fmt.pix.height      = 480;
                fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
                fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

                if (-1 == v4l2capture_xioctl(fd, VIDIOC_S_FMT, &fmt))
                        v4l2capture_errno_exit("VIDIOC_S_FMT");

                /* Note VIDIOC_S_FMT may change width and height. */
        } else {
                /* Preserve original settings as set by v4l2-ctl for example */
                if (-1 == v4l2capture_xioctl(fd, VIDIOC_G_FMT, &fmt))
                        v4l2capture_errno_exit("VIDIOC_G_FMT");
        }

        /* Buggy driver paranoia. */
        min = fmt.fmt.pix.width * 2;
        if (fmt.fmt.pix.bytesperline < min)
                fmt.fmt.pix.bytesperline = min;
        min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
        if (fmt.fmt.pix.sizeimage < min)
                fmt.fmt.pix.sizeimage = min;

        switch (io) {
        case IO_METHOD_READ:
                v4l2capture_init_read(fmt.fmt.pix.sizeimage);
                break;

        case IO_METHOD_MMAP:
                v4l2capture_init_mmap();
                break;

        case IO_METHOD_USERPTR:
                v4l2capture_init_userp(fmt.fmt.pix.sizeimage);
                break;
        }
}

void v4l2capture_close_device(void)
{
        if (-1 == close(fd))
                v4l2capture_errno_exit("close");

        fd = -1;
}

void v4l2capture_open_device(void)
{
        struct stat st;

        if (-1 == stat(dev_name, &st)) {
                fprintf(stderr, "Cannot identify '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }

        if (!S_ISCHR(st.st_mode)) {
                fprintf(stderr, "%s is no device\n", dev_name);
                exit(EXIT_FAILURE);
        }

        fd = open(dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

        if (-1 == fd) {
                fprintf(stderr, "Cannot open '%s': %d, %s\n",
                         dev_name, errno, strerror(errno));
                exit(EXIT_FAILURE);
        }
}

void v4l2capture_usage(FILE *fp, int argc, char **argv)
{
        fprintf(fp,
                 "Usage: %s [options]\n\n"
                 "Version 1.3\n"
                 "Options:\n"
                 "-d | --device name   Video device name [%s]\n"
                 "-h | --help          Print this message\n"
                 "-m | --mmap          Use memory mapped buffers [default]\n"
                 "-r | --read          Use read() calls\n"
                 "-u | --userp         Use application allocated buffers\n"
                 "-o | --output        Outputs stream to stdout\n"
                 "-f | --format        Force format to 640x480 YUYV\n"
                 "-c | --count         Number of frames to grab [%i]\n"
                 "",
                 argv[0], dev_name, frame_count);
}

static const char short_options[] = "d:hmruofc:";

static const struct option
long_options[] = {
        { "device", required_argument, NULL, 'd' },
        { "help",   no_argument,       NULL, 'h' },
        { "mmap",   no_argument,       NULL, 'm' },
        { "read",   no_argument,       NULL, 'r' },
        { "userp",  no_argument,       NULL, 'u' },
        { "output", no_argument,       NULL, 'o' },
        { "format", no_argument,       NULL, 'f' },
        { "count",  required_argument, NULL, 'c' },
        { 0, 0, 0, 0 }
};

/* called directly by the C++ main() */
int v4l2_raw_capture_main(int argc, char *argv[],
        void (*callback_function)(void *, size_t),    // call with each ready buffer
        void (*logger_function)(const char *))        // logging facility
{
        dev_name = "/dev/video0";

        // NOTE:
        // this is being called with argc = 0.  The out_buf variable
        // is already set to 1, and io is set to IO_METHOD_MMAP.
        // The argv[0] string this function is called with is set to
        // the program name (not necessarily the executable name).
        if (argc != 0) for (;;) {
                int idx;
                int c;

                c = getopt_long(argc, argv,
                                short_options, long_options, &idx);

                if (-1 == c)
                        break;

                switch (c) {
                case 0: /* getopt_long() flag */
                        break;

                case 'd':
                        dev_name = optarg;
                        break;

                case 'h':
                        v4l2capture_usage(stderr, argc, argv);
                        exit(EXIT_SUCCESS);

                case 'm':
                        io = IO_METHOD_MMAP;
                        break;

                case 'r':
                        io = IO_METHOD_READ;
                        break;

                case 'u':
                        io = IO_METHOD_USERPTR;
                        break;

                case 'o':
                        out_buf++;
                        break;

                case 'f':
                        force_format++;
                        break;

                case 'c':
                        errno = 0;
                        frame_count = strtol(optarg, NULL, 0);
                        if (errno)
                                v4l2capture_errno_exit(optarg);
                        break;

                default:
                        v4l2capture_usage(stderr, argc, argv);
                        exit(EXIT_FAILURE);
                }
        }

        // The first two function calls have to happen first.
        v4l2capture_set_logger_function(logger_function); // Can be NULL if no callback
        v4l2capture_set_callback_function(callback_function); // Can be NULL if no callback
        v4l2capture_open_device();
        v4l2capture_init_device();
        v4l2capture_start_capturing();
        v4l2capture_mainloop();
        v4l2capture_stop_capturing();
        v4l2capture_uninit_device();
        v4l2capture_close_device();
        return 0;
}

