

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

#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <Utility.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <MainLogger.hpp>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <string>

// Video::vcGlobals statics' definition

std::string     Video::vcGlobals::logChannelName =          "video_capture";
std::string     Video::vcGlobals::logFilelName =            Video::vcGlobals::logChannelName + "_log.txt";
std::string     Video::vcGlobals::output_file =             Video::vcGlobals::logChannelName + ".data";  // Name of file intended for the video frames
Log::Log::Level Video::vcGlobals::loglevel =                Log::Log::Level::eNotice;
std::string     Video::vcGlobals::default_log_level =       Log::Log::toString(Video::vcGlobals::loglevel);
std::string     Video::vcGlobals::log_level =               Video::vcGlobals::default_log_level;
std::string     Video::vcGlobals::config_file_name =        Video::vcGlobals::logChannelName + ".json";
bool            Video::vcGlobals::profiling_enabled =       false;
int             Video::vcGlobals::profile_timeslice_ms =    800;

// Video configuration
std::string     Video::vcGlobals::video_grabber_name =      "v4l2";
bool            Video::vcGlobals::write_frames_to_file =    true;
size_t          Video::vcGlobals::framecount =              200;
std::string     Video::vcGlobals::str_frame_count(std::to_string(framecount));
std::string     Video::vcGlobals::str_dev_name =            "/dev/video0";

// See /usr/include/linux/videodev2.h
enum Video::pxl_formats Video::vcGlobals::pixel_format = Video::pxl_formats::h264;
std::vector<std::string> Video::vcGlobals::pixel_formats_strings ={         // indexed by pixel_format used as an int
                                        "V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2",
                                        "V4L2_PIX_FMT_H264: H264 with start codes"
                                    };

// This function overwrites values in Video::vcGlobals with content from
// the json config file.
//
// We do not use std::endl here to avoid flushing subtleties at the end of each line.
// We count on JsonCpp to throw an exception if the [] access to the node yields an error.
// Caller should try/catch(std::exception)'s.
bool Video::updateInternalConfigsWithJsonValues(std::ostream& strm, const Json::Value& cfg_root)
{
    using namespace Util;

    // logChannelName
    strm << "\nGetting config values from config file " << Video::vcGlobals::config_file_name << "\n";

    Video::vcGlobals::logChannelName = Utility::trim(cfg_root["Config"]["Logger"]["channel-name"].asString());
    strm << "\nFrom JSON:  Set logger channel-name to: " << Video::vcGlobals::logChannelName;

    // logFilelName
    Video::vcGlobals::logFilelName = Utility::trim(cfg_root["Config"]["Logger"]["file-name"].asString());
    strm << "\nFrom JSON:  Set logger file-name to: " << Video::vcGlobals::logFilelName;

    // loglevel and log_level
    Video::vcGlobals::log_level = Utility::trim(cfg_root["Config"]["Logger"]["log-level"].asString());
    strm << "\nFrom JSON:  Set default logger log level to: " << Video::vcGlobals::log_level;

    // video frame grabber
    Video::vcGlobals::video_grabber_name = Utility::trim(cfg_root["Config"]["Video"]["preferred-interface"].asString());
    strm << "\nFrom JSON:  Set default video-frame-grabber to: " << Video::vcGlobals::video_grabber_name;

   // Video::vcGlobals::output_file
    // Raw video output file
    Video::vcGlobals::output_file = Utility::trim(cfg_root["Config"]["App-options"]["output-file"].asString());
    strm << "\nFrom JSON:  Set raw video output file name to: " << Video::vcGlobals::output_file;







    return true;
}

