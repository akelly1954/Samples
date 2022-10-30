#pragma once

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

#include <json/json.h>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <ostream>
#include <sstream>
#include <string>

namespace Video
{
    // Pixel formats: see /usr/include/linux/videodev2.h
    enum pxl_formats {
        yuyv = 0,
        h264
    };

    struct vcGlobals
    {
        static bool log_initialization_info;
        static std::string logChannelName;
        static std::string logFilelName;
        static std::string output_file;
        static std::string output_process;
        static bool use_other_proc;
        static Log::Log::Level loglevel;
        static std::string log_level;
        static bool profiling_enabled;
        static int  profile_timeslice_ms;
        static bool capture_finished;
        static bool capture_pause;
        static std::string config_file_name;

        // Video configuration
        static std::string video_grabber_name;
        static size_t framecount;
        static bool write_frames_to_file;
        static bool write_frames_to_process;
        static std::string str_frame_count;
        static std::string str_dev_name;
        static bool proc_redir;
        static std::string redir_filename;

        // Indexed by enum pxl_formats values
        // has a string description for each enum value
        // See /usr/include/linux/videodev2.h
        static enum pxl_formats pixel_format;
        static std::vector<std::string> pixel_formats_strings;
    };

// This function overwrites values in Video::vcGlobals with content from
// the json config file.
bool updateInternalConfigsWithJsonValues(std::ostream& strm, const Json::Value& cfg_root);

} // end of namespace Video

