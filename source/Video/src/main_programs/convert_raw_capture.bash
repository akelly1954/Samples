#!/bin/bash

# This converts the H264 output from video_capture.dd.data to video_capture_dd.mp4.

ffmpeg -f h264 -i video_capture.dd.data -vcodec copy video_capture_dd.mp4.

# Alternatively, this converts the YUYV output from video_capture.dd.data to video_capture_dd.mp4.

ffmpeg -f rawvideo -vcodec rawvideo -s 640x480 -r 25 -pix_fmt yuyv422 -i video_capture.dd.data \
       -c:v libx264 -preset ultrafast -qp 0 video_capture_dd.mp4 
       