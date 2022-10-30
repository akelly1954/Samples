#!/bin/bash

# Converts the H264 output from video_capture.dd.data to video_capture_dd.mp4 in mp4 format.

ffmpeg -f h264 -i video_capture.dd.data -vcodec copy video_capture_dd.mp4