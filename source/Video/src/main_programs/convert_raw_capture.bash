#!/bin/bash

# Converts the H264 output from main_v4l2_raw_capture to .mp4 format.

ffmpeg -f h264 -i v4l2_raw_capture.data -vcodec copy v4l2_raw_capture.mp4

