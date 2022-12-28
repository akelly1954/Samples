

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
bool            Video::vcGlobals::log_initialization_info = false;
std::string     Video::vcGlobals::logChannelName =          "video_capture";
std::string     Video::vcGlobals::logFilelName =            Video::vcGlobals::logChannelName + "_log.txt";
std::string     Video::vcGlobals::output_file =             Video::vcGlobals::logChannelName + ".data"; // Name of file intended for the video frames
std::string     Video::vcGlobals::output_process =          "date > /dev/null 2>& 1";                   // command to pipe frames to (for popen() - use the json file to set correctly).
bool            Video::vcGlobals::use_other_proc =          false;
Log::Log::Level Video::vcGlobals::loglevel =                Log::Log::Level::eNotice;
std::string     Video::vcGlobals::log_level =               Log::Log::toString(Video::vcGlobals::loglevel);
std::string     Video::vcGlobals::config_file_name =        Video::vcGlobals::logChannelName + ".json";
bool            Video::vcGlobals::profiling_enabled =       false;
int             Video::vcGlobals::profile_timeslice_ms =    800;

// Video configuration
std::string     Video::vcGlobals::video_grabber_name =      "v4l2";
bool            Video::vcGlobals::write_frames_to_file =    true;
bool            Video::vcGlobals::write_frames_to_process = false;
size_t          Video::vcGlobals::framecount =              200;
std::string     Video::vcGlobals::str_frame_count(std::to_string(framecount));
std::string     Video::vcGlobals::str_dev_name =            "/dev/video0";
std::string     Video::vcGlobals::str_plugin_file_name =    "./undefined.so";
bool            Video::vcGlobals::proc_redir =              true;
std::string     Video::vcGlobals::redir_filename =          "/dev/null";
bool            Video::vcGlobals::test_suspend_resume =     false;


// See /usr/include/linux/videodev2.h for the descriptive strings in the vector<>
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

    std::string preferredInterface = cfg_root["Config"]["Video"]["preferred-interface"].asString();
    // logChannelName
    strm << "\nFrom JSON:  Getting available pixel formats for interface "
         << "\"" << preferredInterface << "\":\n";

    // Create the list
    std::string pixlist = "        {  ";
    Json::Value pixfmt = cfg_root["Config"]
                                  ["Video"]
                                   ["frame-capture"]
                                    [preferredInterface]
                                     ["pixel-format"];

    for (auto itr = pixfmt.begin(); itr != pixfmt.end(); itr++)
    {
        std::string itrkey = itr.key().asString();
        pixlist += (itrkey + "  ");
    }
    pixlist += "  }";

    strm << pixlist;

    // End of list creation

    Video::vcGlobals::logChannelName = Utility::trim(cfg_root["Config"]["Logger"]["channel-name"].asString());
    strm << "\nFrom JSON:  Set logger channel-name to: " << Video::vcGlobals::logChannelName;

    // logFilelName
    Video::vcGlobals::logFilelName = Utility::trim(cfg_root["Config"]["Logger"]["file-name"].asString());
    strm << "\nFrom JSON:  Set logger file-name to: " << Video::vcGlobals::logFilelName;

    // loglevel and log_level
    Video::vcGlobals::log_level = Utility::trim(cfg_root["Config"]["Logger"]["log-level"].asString());
    strm << "\nFrom JSON:  Set default logger log level to: " << Video::vcGlobals::log_level;

    // Enable writing raw video frames to output file
    bool enable_write_to_file = !(cfg_root["Config"]["App-options"]["write-to-file"].asInt() == 0);
    Video::vcGlobals::write_frames_to_file = enable_write_to_file;
    strm << "\nFrom JSON:  Enable writing raw video frames to output file: " << (enable_write_to_file? "true" : "false");

    // Raw video output file
    Video::vcGlobals::output_file = Utility::trim(cfg_root["Config"]["App-options"]["output-file"].asString());
    strm << "\nFrom JSON:  Set raw video output file name to: " << Video::vcGlobals::output_file;

    // Enable writing raw video frames to process
    bool enable_write_to_process = !(cfg_root["Config"]["App-options"]["write-to-process"].asInt() == 0);
    Video::vcGlobals::write_frames_to_process = enable_write_to_process;
    strm << "\nFrom JSON:  Enable writing raw video frames to process: " << (enable_write_to_process? "true" : "false");

    // TODO: Consider adding an explicit config entry in the json file equivalent to vcGlobals::redir_filename.
    // Currently, it can only be specified on the command line together with the -proc-redir command line option.

    // Enable profiling operations
    bool enable_profiling = !(cfg_root["Config"]["App-options"]["profiling"].asInt() == 0);
    Video::vcGlobals::profiling_enabled = enable_profiling;
    strm << "\nFrom JSON:  Enable profiling: " << (enable_profiling? "true" : "false");

    // Milliseconds between profile snapshots
    Video::vcGlobals::profile_timeslice_ms = cfg_root["Config"]["App-options"]["profile-timeslice-ms"].asInt();
    strm << "\nFrom JSON:  Set milliseconds between profile snapshots to: " << Video::vcGlobals::profile_timeslice_ms;

    // video frame grabber
    Video::vcGlobals::video_grabber_name = Utility::trim(cfg_root["Config"]["Video"]["preferred-interface"].asString());
    strm << "\nFrom JSON:  Set default video-frame-grabber to: " << Video::vcGlobals::video_grabber_name;

    // video grabber frame count
    Video::vcGlobals::framecount = cfg_root["Config"]["Video"]["frame-count"].asInt();
    strm << "\nFrom JSON:  Set number of frames to grab (framecount) to: " << Video::vcGlobals::framecount;

    ///////////// Specific video-grabber section (i.e. v4l2, opencv, etc) /////////////////

    strm << "\nFrom JSON:  " << Video::vcGlobals::video_grabber_name << " is labeled as: "
            << cfg_root["Config"]["Video"]["frame-capture"][Video::vcGlobals::video_grabber_name]["name"].asString();

    Video::vcGlobals::str_dev_name = cfg_root["Config"]
                                              ["Video"]
                                               ["frame-capture"]
                                                [Video::vcGlobals::video_grabber_name]
                                                 ["device-name"].asString();
    strm << "\nFrom JSON:  Set " << Video::vcGlobals::video_grabber_name << " device name to " << Video::vcGlobals::str_dev_name;

    // plugin-file-name - Video::vcGlobals::str_plugin_file_name
    Video::vcGlobals::str_plugin_file_name = cfg_root["Config"]
                                                  ["Video"]
                                                   ["frame-capture"]
                                                    [Video::vcGlobals::video_grabber_name]
                                                     ["plugin-file-name"].asString();
    strm << "\nFrom JSON:  Set grabber plugin file name to " << Video::vcGlobals::str_plugin_file_name;

    // Video::vcGlobals::pixel_format is either "h264" or "yuyv"
    std::string pixelFormat = cfg_root["Config"]
                                       ["Video"]
                                        ["frame-capture"]
                                         [Video::vcGlobals::video_grabber_name]
                                          ["preferred-pixel-format"].asString();
    if (pixelFormat == "h264")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::h264;
    }
    else if (pixelFormat == "yuyv")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::yuyv;
    }
    else
    {
        throw std::runtime_error(std::string("ERROR in Video::updateInternalConfigsWithJsonValues(): Invalid pixel format: ") + pixelFormat + " specified");
    }
    strm << "\nFrom JSON:  Set " << Video::vcGlobals::video_grabber_name << " pixel format to "
            << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_format];

    // Raw video output file
    Video::vcGlobals::output_process = cfg_root["Config"]
                                                ["Video"]
                                                 ["frame-capture"]
                                                  [Video::vcGlobals::video_grabber_name]
                                                   ["pixel-format"]
                                                    [pixelFormat]
                                                     ["output-process"].asString();
    strm << "\nFrom JSON:  Set raw video output process command to: " << Video::vcGlobals::output_process;

    return true;
}

// adds double quotes to string - hello to "hello"
std::string adq(const std::string& str)
{
    std::string rstr = "\"";
    rstr += str;
    rstr +="\"";
    return rstr;
}

std::string rsb(bool x)
{
    return ((x)? "true": "false");
}

void Video::vcGlobals::print_globals(std::ostream& strm)
{
    using namespace Video;

    strm << "\n\nCURRENT RUNTIME CONFIGURATION DETAILS:" << "\n\n"
            "   For each item detailed below, the actual runtime value of the item is displayed,\n"
            "   Followed by the command line options for it, followed by information about where\n"
            "   the detail is modified and set: the \"struct vcGlobals\" member name, followed\n"
            "   by the json location in the config file.\n"
            "\n"
            "   PLEASE NOTE: The json config file name is always the logger channel name, \n"
            "   with \".json\" appended to it.\n"
         << "\n";

    strm << "Logger channel-name:      " << adq(Video::vcGlobals::logChannelName) << "\n"
         << "    command line flag(s): NONE: can only be set in " << adq(Video::vcGlobals::logChannelName + ".json") << "\n"
         << "    in vcGlobals object:  Video::vcGlobals::logChannelName\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"channel-name\"]\n"
         << "\n";

    strm << "Log file name:            " << adq(Video::vcGlobals::logFilelName) << "\n"
         << "    command line flag(s): NONE: can only be set in " << adq(Video::vcGlobals::logChannelName + ".json") << "\n"
         << "    in object:            Video::vcGlobals::logFilelName\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"channel-name\"]\n"
         << "\n";

    strm << "Log level:                " << adq(Video::vcGlobals::log_level) << "\n"
         << "    command line flag:    [ -lg log-level ]\n"
         << "    in object:            Video::vcGlobals::log_level (as string)\n"
         << "                          Video::vcGlobals::loglevel (as enum)\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"log-level\"]\n"
         << "\n";

    strm << "Enable profiling:         " << rsb(Video::vcGlobals::profiling_enabled) << ", " << Video::vcGlobals::profile_timeslice_ms << " milliseconds per slice\n"
         << "    command line flag:    [ -pr [ timeslice_ms ] ]\n"
         << "    in object:            Video::vcGlobals::profiling_enabled\n"
         << "                          Video::vcGlobals::profile_timeslice_ms\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"profiling\"]\n"
         << "                          Root[\"Config\"][\"App-options\"][\"profile-timeslice-ms\"]\n"
         << "\n";

    strm << "Enable write to file:     " << rsb(Video::vcGlobals::write_frames_to_file) << ", output file: " << adq(Video::vcGlobals::output_file) << "\n"
         << "    command line flag:    [ -fn [ file-name ] ]\n"
         << "    in object:            Video::vcGlobals::write_frames_to_file\n"
         << "                          Video::vcGlobals::output_file\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"write-to-file\"]\n"
         << "                          Root[\"Config\"][\"App-options\"][\"output-file\"]\n"
         << "\n";

    strm << "Enable write to process:  " << rsb(Video::vcGlobals::write_frames_to_process) << ", stderr redirected to: " << adq(Video::vcGlobals::redir_filename) << "\n"
         << "    command line flag:    [ -proc-redir [ file ]]\n"
         << "    in object:            Video::vcGlobals::write_frames_to_process\n"
         << "                          Video::vcGlobals::redir_filename\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"write-to-process\"]\n"
         << "                          (the stderr redirection file-name does not currently exist in the json file).\n"
         << "\n";

    strm << "Frame grabber name:       " << adq(Video::vcGlobals::video_grabber_name) << "\n"
         << "    command line flag:    [ -fg video-grabber ] \n"
         << "    in object:            Video::vcGlobals::video_grabber_name\n"
         << "    in json config:       Root[\"Config\"][\"Video\"][\"preferred-interface\"]\n"
         << "\n";

    strm << "Frame count:              " << std::to_string(Video::vcGlobals::framecount)
         <<                            (Video::vcGlobals::framecount == 0? " (continuous)" : " ") << "\n"
         << "    command line flag:    [ -fc frame-count ] \n"
         << "    in object:            Video::vcGlobals::framecount\n"
         << "    in json config:       Root[\"Config\"][\"Video\"][\"frame-count\"]\n"
         << "\n";

    strm << "SEE ALSO: The displayed help shown when running " << adq("main_video_capture --help") << "\n";

#if 0

    strm << " " << Video::vcGlobals::video_grabber_name << " is labeled as: "
            << Root["Config"]["Video"]["frame-capture"][Video::vcGlobals::video_grabber_name]["name"].asString();

    Video::vcGlobals::str_dev_name = Root["Config"]
                                              ["Video"]
                                               ["frame-capture"]
                                                [Video::vcGlobals::video_grabber_name]
                                                 ["device-name"].asString();
    strm << "" << Video::vcGlobals::video_grabber_name << " device name to " << Video::vcGlobals::str_dev_name;

    // plugin-file-name - Video::vcGlobals::str_plugin_file_name
    Video::vcGlobals::str_plugin_file_name = Root["Config"]
                                                  ["Video"]
                                                   ["frame-capture"]
                                                    [Video::vcGlobals::video_grabber_name]
                                                     ["plugin-file-name"].asString();
    strm << "grabber plugin file name to " << Video::vcGlobals::str_plugin_file_name;

    // Video::vcGlobals::pixel_format is either "h264" or "yuyv"
    std::string pixelFormat = Root["Config"]
                                       ["Video"]
                                        ["frame-capture"]
                                         [Video::vcGlobals::video_grabber_name]
                                          ["preferred-pixel-format"].asString();
    if (pixelFormat == "h264")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::h264;
    }
    else if (pixelFormat == "yuyv")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::yuyv;
    }
    else
    {
        throw std::runtime_error(std::string("ERROR in Video::updateInternalConfigsWithJsonValues(): Invalid pixel format: ") + pixelFormat + " specified");
    }
    strm << "" << Video::vcGlobals::video_grabber_name << " pixel format to "
            << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_format];

    // Raw video output file
    Video::vcGlobals::output_process = Root["Config"]
                                                ["Video"]
                                                 ["frame-capture"]
                                                  [Video::vcGlobals::video_grabber_name]
                                                   ["pixel-format"]
                                                    [pixelFormat]
                                                     ["output-process"].asString();
    strm << "raw video output process command to: " << Video::vcGlobals::output_process;


#endif // 0

}


