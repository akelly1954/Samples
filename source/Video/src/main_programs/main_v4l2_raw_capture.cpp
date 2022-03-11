/*
 * Adapted from kernel.org/doc/html/v4.11/media/uapi/v4l/capture.c.html 3-10-2022
 */

/*
 * This is the C++ main fronting for the C main (called here v4l2_raw_capture_main()).
 *
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see https://linuxtv.org/docs.php for more information
 */

#include <v4l2_raw_capture.h>

int main(int argc, char *argv[])
{
    return v4l2_raw_capture_main(argc, argv);
}

