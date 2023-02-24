
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
#include <ConfigSingleton.hpp>
#include <video_capture_globals.hpp>
#include <vidcap_capture_thread.hpp>
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
bool            Video::vcGlobals::log_initialization_info =     false;
std::string     Video::vcGlobals::logChannelName =              "video_capture";
std::string     Video::vcGlobals::logFilelName =                Video::vcGlobals::logChannelName + "_log.txt";
std::string     Video::vcGlobals::output_file =                 Video::vcGlobals::logChannelName + ".data"; // Name of file intended for the video frames
std::string     Video::vcGlobals::output_process =              "date > /dev/null 2>& 1";                   // command to pipe frames to (for popen() - use the json file to set correctly).
bool            Video::vcGlobals::use_other_proc =              false;
bool            Video::vcGlobals::display_runtime_config =      false;
std::string     Video::vcGlobals::runtime_config_output_file =  Video::vcGlobals::logChannelName + "_runtime_config.txt";
Log::Log::Level Video::vcGlobals::loglevel =                    Log::Log::Level::eNotice;
std::string     Video::vcGlobals::log_level =                   Log::Log::toString(Video::vcGlobals::loglevel);
std::string     Video::vcGlobals::config_file_name =            Video::vcGlobals::logChannelName + ".json";
bool            Video::vcGlobals::profiling_enabled =           false;
int             Video::vcGlobals::profile_timeslice_ms =        800;

// Video configuration
std::string     Video::vcGlobals::video_grabber_name =          "v4l2";
bool            Video::vcGlobals::write_frames_to_file =        true;
bool            Video::vcGlobals::write_frames_to_process =     false;
size_t          Video::vcGlobals::framecount =                  200;
std::string     Video::vcGlobals::str_frame_count(std::to_string(framecount));
std::string     Video::vcGlobals::str_dev_name =                "/dev/video0";
std::string     Video::vcGlobals::str_plugin_file_name =        "./undefined.so";
bool            Video::vcGlobals::proc_redir =                  true;
std::string     Video::vcGlobals::redir_filename =              "/dev/null";
bool            Video::vcGlobals::test_suspend_resume =         false;


// See /usr/include/linux/videodev2.h for the descriptive strings in the vector<>
enum Video::pxl_formats Video::vcGlobals::pixel_fmt = Video::pxl_formats::h264;
std::vector<std::string> Video::vcGlobals::pixel_formats_strings ={         // indexed by pixel_fmt used as an int
                                        "V4L2_PIX_FMT_YYUV: 16bit YUV 4:2:2",
                                        "V4L2_PIX_FMT_H264: H264 with start codes"
                                    };


//////////////////////////////////////////////////////////////////////////////
// static methods dealing with the frame count
//////////////////////////////////////////////////////////////////////////////

// static size_t framecount;
// static std::string str_frame_count;
void Video::vcGlobals::set_framecount(int count)
{
    Video::vcGlobals::framecount = count;
    Video::vcGlobals::str_frame_count = std::to_string(count);
}
void Video::vcGlobals::set_framecount(std::string strcount)
{
    Video::vcGlobals::str_frame_count = strcount;
    Video::vcGlobals::framecount = static_cast<int>(strtoul(strcount.c_str(), NULL, 10));
}

//////////////////////////////////////////////////////////////////////////////
// struct pixel_format members and methods
//////////////////////////////////////////////////////////////////////////////

std::map<std::string,std::string> Video::pixel_format::s_pixformat;
bool Video::pixel_format::s_pix_initialized = false;
std::string Video::pixel_format::s_video_interface;

// This method probes the JSON object for all the pixel formats
// allowed in the preferred-interface selected in the "Video" section
// of the JSON config file.
bool Video::pixel_format::pixfmt_setup(const Json::Value& cfg_root)
{
    using namespace Util;

    s_video_interface = cfg_root["Config"]["Video"]["preferred-interface"].asString();

    Json::Value pixfmt = cfg_root["Config"]
                                  ["Video"]
                                   ["frame-capture"]
                                    [s_video_interface]
                                     ["pixel-format"];

    for (auto itr = pixfmt.begin(); itr != pixfmt.end(); itr++)
    {
        std::string itrkey = itr.key().asString();
        if (itrkey != "other")
        {
            std::string fdesc = (*itr)["format-description"].asString();
            s_pixformat[itrkey] = fdesc;
        }
    }

    // DEBUG: Check the map
    // for (auto mitr = s_pixformat.begin(); mitr != s_pixformat.end(); mitr++)
    // {
    //     std::cerr << "       map[" << Utility::string_enquote(mitr->first) << "] = " << Utility::string_enquote(mitr->second) << std::endl;
    // }
    s_pix_initialized = true;
    return true;
}

// overload
bool Video::pixel_format::pixfmt_setup(void)
{
    Json::Value& Root = Config::ConfigSingleton::instance()->JsonRoot();
    return Video::pixel_format::pixfmt_setup(Root);
}

std::string Video::pixel_format::pixfmt_description(std::string pixfmtname)
{
    if (! s_pix_initialized) return "";
    auto mitr = s_pixformat.find(pixfmtname);
    if (mitr != s_pixformat.end()) return mitr->second;
    return "";
}

void Video::pixel_format::displayPixelFormatConfig(std::ostream& ostrm)
{
    using namespace Util;

    ostrm << "Available pixel formats for interface " << Utility::string_enquote(Video::pixel_format::s_video_interface) << ":\n";
    for (auto mitr = Video::pixel_format::s_pixformat.begin(); mitr != Video::pixel_format::s_pixformat.end(); mitr++)
    {
        ostrm << "     " << Utility::string_enquote(mitr->first) << " pixel-format -> " << Utility::string_enquote(mitr->second) << "\n";
    }
}


//////////////////////////////////////////////////////////////////////////////
// function updateInternalConfigsWithJsonValues
//////////////////////////////////////////////////////////////////////////////

// This function overwrites values in Video::vcGlobals with content from
// the json config file.
//
// We do not use std::endl here to avoid flushing subtleties at the end of each line.
// We count on JsonCpp to throw an exception if the [] access to the node yields an error.
// Caller should try/catch(std::exception)'s.
bool Video::updateInternalConfigsWithJsonValues(std::ostream& strm, const Json::Value& cfg_root)
{
    using namespace Util;

    // TODO: This block has to move from here until after plugin_factory loads up the plugin
    // Video::pixel_format::pixfmt_setup(cfg_root);
    // strm << "\nFrom JSON:  ";
    // Video::pixel_format::displayPixelFormatConfig(strm);
    // TODO: The above block has to move from here until after plugin_factory loads up the plugin

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
    int ct = cfg_root["Config"]["Video"]["frame-count"].asInt();
    Video::vcGlobals::set_framecount(ct);
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

    // Video::vcGlobals::pixel_fmt is either "h264" or "yuyv"
    std::string pixelFormat = cfg_root["Config"]
                                       ["Video"]
                                        ["frame-capture"]
                                         [Video::vcGlobals::video_grabber_name]
                                          ["preferred-pixel-format"].asString();
    if (pixelFormat == "h264")
    {
        Video::vcGlobals::pixel_fmt = Video::pxl_formats::h264;
    }
    else if (pixelFormat == "yuyv")
    {
        Video::vcGlobals::pixel_fmt = Video::pxl_formats::yuyv;
    }
    else
    {
        throw std::runtime_error(std::string("ERROR in Video::updateInternalConfigsWithJsonValues(): Invalid pixel format: ") + pixelFormat + " specified");
    }
    strm << "\nFrom JSON:  Set " << Video::vcGlobals::video_grabber_name << " pixel format to "
            << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_fmt];

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

void Video::vcGlobals::print_globals(std::ostream& strm)
{
    using namespace Video;
    using Util::Utility;

    Json::Value& Root = Config::ConfigSingleton::instance()->JsonRoot();

    strm << "\n\n"
         << "CURRENT RUNTIME CONFIGURATION DETAILS:\n"
         << "\n"
         << "   For each item detailed below, the actual runtime value of the item is displayed,\n"
         << "   Followed by the command line options for it, followed by information about where\n"
         << "   the detail is modified and set: the \"struct vcGlobals\" member name, followed\n"
         << "   by the json location in the config file. \n"
         << "\n"
         << "   PLEASE NOTE that for any configuration detail, the value stored in the " << Utility::string_enquote("vcGlobals") << "\n"
         << "   object is the definitive runtime value. Details that are not represented in " << Utility::string_enquote("vcGlobals") << "\n"
         << "   should be found in the json file.\n"
         << "\n"
         << "   PLEASE NOTE: The json config file name is always the logger channel name, \n"
         << "   with \".json\" appended to it.\n"
         << "\n";

    strm << "Logger channel-name:      " << Utility::string_enquote(Video::vcGlobals::logChannelName) << "\n"
         << "    command line flag(s): NONE: can only be set in " << Utility::string_enquote(Video::vcGlobals::logChannelName + ".json") << "\n"
         << "    in vcGlobals object:  vcGlobals::logChannelName\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"channel-name\"]\n"
         << "\n";

    strm << "Log file name:            " << Utility::string_enquote(vcGlobals::logFilelName) << "\n"
         << "    command line flag(s): NONE: can only be set in " << Utility::string_enquote(vcGlobals::logChannelName + ".json") << "\n"
         << "    in object:            vcGlobals::logFilelName\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"channel-name\"]\n"
         << "\n";

    strm << "Log level:                " << Utility::string_enquote(vcGlobals::log_level) << "\n"
         << "    command line flag:    [ -lg log-level ]\n"
         << "    in object:            vcGlobals::log_level (as string)\n"
         << "                          vcGlobals::loglevel (as enum)\n"
         << "    in json config:       Root[\"Config\"][\"Logger\"][\"log-level\"]\n"
         << "\n";

    strm << "Enable profiling:         " << Utility::stringify_bool(vcGlobals::profiling_enabled) << ", " << vcGlobals::profile_timeslice_ms << " milliseconds per slice\n"
         << "    command line flag:    [ -pr [ timeslice_ms ] ]\n"
         << "    in object:            vcGlobals::profiling_enabled\n"
         << "                          vcGlobals::profile_timeslice_ms\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"profiling\"]\n"
         << "                          Root[\"Config\"][\"App-options\"][\"profile-timeslice-ms\"]\n"
         << "\n";

    strm << "Enable write to file:     " << Utility::stringify_bool(vcGlobals::write_frames_to_file) << ", output file: " << Utility::string_enquote(vcGlobals::output_file) << "\n"
         << "    command line flag:    [ -fn [ file-name ] ]\n"
         << "    in object:            vcGlobals::write_frames_to_file\n"
         << "                          vcGlobals::output_file\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"write-to-file\"]\n"
         << "                          Root[\"Config\"][\"App-options\"][\"output-file\"]\n"
         << "\n";

    strm << "Enable write to process:  " << Utility::stringify_bool(vcGlobals::write_frames_to_process) << ", stderr redirected to: " << Utility::string_enquote(vcGlobals::redir_filename) << "\n"
         << "    command line flag:    [ -proc-redir [ file ]]\n"
         << "    in object:            vcGlobals::write_frames_to_process\n"
         << "                          vcGlobals::redir_filename\n"
         << "    in json config:       Root[\"Config\"][\"App-options\"][\"write-to-process\"]\n"
         << "                          (the stderr redirection file-name does not currently exist in the json file).\n"
         << "\n";

    strm << "Frame grabber name:       " << Utility::string_enquote(vcGlobals::video_grabber_name) << "\n"
         << "    command line flag:    [ -fg video-grabber ] \n"
         << "    in object:            vcGlobals::video_grabber_name\n"
         << "    in json config:       Root[\"Config\"][\"Video\"][\"preferred-interface\"]\n"
         << "\n";

    strm << "Frame count:              " << std::to_string(vcGlobals::framecount)
         <<                            (vcGlobals::framecount == 0? " (continuous)" : " ") << "\n"
         << "    command line flag:    [ -fc frame-count ] \n"
         << "    in object:            vcGlobals::framecount\n"
         << "    in json config:       Root[\"Config\"][\"Video\"][\"frame-count\"]\n"
         << "\n";

    strm << "\nThe following section displays the runtime CONFIGURATION DETAILS OF \n"
         << "THE SPECIFIC PLUGIN which is already loaded and running at this time.\n\n"
         << "   For each item detailed below, the runtime value of the item is displayed,\n"
         << "   followed by the command line options for it, followed by information about where\n"
         << "   the detail is modified and set: the \"struct vcGlobals\" member name, followed\n"
         << "   by the json location in the config file.\n"
         << "\n";

    // get a shortcut to the plugin section, and then use that to access individual members
    const Json::Value& frameRoot = Root["Config"]["Video"]["frame-capture"][vcGlobals::video_grabber_name];
    std::string pluginlabel = frameRoot["name"].asString();
    std::string pixformat = frameRoot["preferred-pixel-format"].asString();

    // For debugging:
    // std::cerr << "plugin label: " << pluginlabel << std::endl;
    // std::cerr << "pixel format : " << pixformat << std::endl;
    // std::string outproc = frameRoot["pixel-format"][ frameRoot["preferred-pixel-format"].asString() ]["output-process"].asString();
    // std::string outproc = frameRoot["pixel-format"][pixformat]["output-process"].asString();
    // std::string outproc = Root["Config"]["Video"]["frame-capture"][vcGlobals::video_grabber_name]["pixel-format"][pixformat]["output-process"].asString();
    // std::cerr << "output process : " << outproc << std::endl;
    // std::string pluginlabel = Root["Config"]["Video"]["frame-capture"][vcGlobals::video_grabber_name]["name"].asString();

    strm << "The loaded plugin is: " << Utility::string_enquote(vcGlobals::str_plugin_file_name) << ".\n"
         << "\n"
         << "   In order to get to specific json \"node\" associated with the plugin, as well as values \n"
         << "   within the \"node\" itself, we use several variables available from within the program in \n"
         << "   the vcGlobals object, and/or the internal representation of the json file itself:\n"
         << "\n"
         << "c++:        Video::vcGlobals::video_grabber_name - the preferred (requested) video grabber interface\n"
         << "\n"
         << "c++/json:   Root[\"Config\"][\"Video\"][\"frame-capture\"][vcGlobals::video_grabber_name] - this is the\n"
         << "            json NODE (section) that has all the different details for the loaded plugin.\n"
         << "\n"
         << "As a convenience, the above c++/json line which obtains the json section can be used to access \n"
         << "individual members of the node like this:\n"
         << "\n"
         << "       const Json::Value& frameRoot = Root[\"Config\"][\"Video\"][\"frame-capture\"][vcGlobals::video_grabber_name]; \n"
         << "\n"
         << "       std::string pluginlabel = frameRoot[\"name\"].asString(); \n"
         << "       std::string pluginfilename = frameRoot[\"plugin-file-name\"].asString(); \n"
         << "       std::string pixformat = frameRoot[\"preferred-pixel-format\"].asString(); \n"
         << "       std::string outproc = frameRoot[\"pixel-format\"][pixformat][\"output-process\"].asString(); \n"
         << "\n"
         << "As a cute convenience (not recommended as it is confusing to some), the \"outproc\" assignment \n"
         << "could be made like this: \n"
         << "\n"
         << "std::string outproc = frameRoot[\"pixel-format\"][ frameRoot[\"preferred-pixel-format\"].asString() ][\"output-process\"].asString(); \n"
         << "\n"
         << "    Please remember that in any case, if there is a valid \"vcGlobals\" entry for the detail being addressed it\n"
         << "    SHOULD be used, rather than the json value (true for all of the above except for \"pluginlabel\" at this time). \n"
         << "\n";

    strm << "    //////////////////////////////////////////////////////////////////////////////////////////////\n"
         << "    // \n"
         << "    // So, for the currently running instance of the program, \n"
         << "    // using:   const Json::Value& frameRoot = Root[\"Config\"][\"Video\"][\"frame-capture\"][" << Utility::string_enquote(vcGlobals::video_grabber_name) << "]; \n"
         << "    // and:     std::string pixformat = frameRoot[" << Utility::string_enquote(pixformat) << "].asString(); \n"
         << "    // \n"
         << "    //////////////////////////////////////////////////////////////////////////////////////////////\n"
         << "\n";

    strm << "    Loaded vidcap plugin: " << Utility::string_enquote(vcGlobals::str_plugin_file_name) << " \n"
         << "    command line flag:    NONE: can only be set in " << Utility::string_enquote(vcGlobals::logChannelName + ".json") << " before runtime.\n"
         << "    in object:            vcGlobals::str_plugin_file_name\n"
         << "    in json config:       frameRoot[\"plugin-file-name\"] \n"
         << "\n";

    // std::string pluginlabel = frameRoot["name"].asString();

    strm << "    Plugin label:         " << Utility::string_enquote(frameRoot["name"].asString()) << " (mostly unused plugin type identifier) \n"
         << "    command line flag:    NONE: can only be set in " << Utility::string_enquote(vcGlobals::logChannelName + ".json") << " before runtime.\n"
         << "    in object:            none\n"
         << "    in json config:       frameRoot[\"name\"] \n"
         << "\n";

    strm << "    Camera device name:   " << Utility::string_enquote(vcGlobals::str_dev_name) << "\n"
         << "    command line flag:    [ -dv video-device ]\n"
         << "    in object:            vcGlobals::str_dev_name\n"
         << "    in json config:       frameRoot[\"device-name\"].asString(); \n"
         << "\n";

    strm << "    Current pixel format: " << Utility::string_enquote(frameRoot["preferred-pixel-format"].asString()) << " - description: " << vcGlobals::pixel_formats_strings[vcGlobals::pixel_fmt] << "\n"
         << "    command line flag:    [ -pf pixel-format ]\n"
         << "    in object:            vcGlobals::pixel_fmt (enum)\n"
         << "                          (vcGlobals::pixel_formats_strings (string vector - converts enum to string)\n"
         << "    in json config:       frameRoot[\"preferred-pixel-format\"].asString(); \n"
         << "\n";

    strm << "    Use \"other process\":  " << Utility::stringify_bool(vcGlobals::use_other_proc) << " (output-process set in JSON file to: " << Utility::string_enquote(frameRoot["pixel-format"]["other"]["output-process"].asString()) << ") \n"
         << "    command line flag:    [ -use-other-proc ]\n"
         << "    in object:            vcGlobals::use_other_proc \n"
         << "    in json config:       frameRoot[\"pixel-format\"][\"other\"][\"output-process\"].asString(); \n"
         << "\n";

    strm << "    Redirect proc stderr: " << Utility::stringify_bool(vcGlobals::proc_redir) << " (stderr of popen process output to go to: " << Utility::string_enquote(vcGlobals::redir_filename) << ") \n"
         << "    command line flag:    [ -proc-redir [ file ] ] \n"
         << "    in object:            vcGlobals::proc_redir (bool) \n"
         << "                          vcGlobals::redir_filename (string) \n"
         << "    in json config:       none (command line only) \n"
         << "\n";

    strm << "    //////////////////////////////////////////////////////////////////////////////////////////////\n"
         << "    // \n"
         << "    // For the currently running instance of the program, using the configuration established, \n"
         << "    // the character string used to invoke the process that handles video frames using popen(): \n"
         << "    // \n"
         << "    //////////////////////////////////////////////////////////////////////////////////////////////\n"
         << "\n";

    strm << "    Popen process string: " << Utility::string_enquote( VideoCapture::video_plugin_base::popen_process_string ) << " \n"
         << "\n";

    Video::pixel_format::displayPixelFormatConfig(strm);

    strm << "\nSEE ALSO: The displayed help shown when running " << Utility::string_enquote("main_video_capture --help") << "\n\n";

}

// Open/truncate the output file that will hold runtime-config info
FILE * Video::vcGlobals::create_runtime_conf_output_file(const std::string& cmdline)
{
    using namespace Video;
    using Util::Utility;

    int errnocopy = 0;
    FILE *output_stream = NULL;

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if ((output_stream = ::fopen (Video::vcGlobals::runtime_config_output_file.c_str(), "w+")) == NULL)
    {
        errnocopy = errno;
        loggerp->error() << "Cannot create/truncate runtime config output file " << Utility::string_enquote(vcGlobals::runtime_config_output_file) << ": " << Utility::get_errno_message(errnocopy);
        return NULL;
    }

    loggerp->debug() << "Created/truncated runtime config output file " << Utility::string_enquote(vcGlobals::output_file);

    char outstr[200];
    time_t t;
    struct tm *tmp;

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp) strftime(outstr, sizeof(outstr), "%Y-%m-%d %X", tmp);

    const std::string ostrng = std::string("\nRuntime date/time: ") + outstr + "\nCommand line: " + cmdline + "\n";
    write_to_runtime_conf_file(output_stream, ostrng);
    return output_stream;   // Check for NULL
}

size_t Video::vcGlobals::write_to_runtime_conf_file(FILE *filestream, const std::string& infostring)
{
    using Util::Utility;

    size_t elementswritten = std::fwrite(infostring.c_str(), sizeof(const char), infostring.length(), filestream);
    int errnocopy = 0;
    size_t byteswritten = elementswritten * sizeof(const char);

    auto loggerp = Util::UtilLogger::getLoggerPtr();

    if (byteswritten != infostring.length())
    {
        loggerp->error() << "vcGlobals::write_to_runtime_conf_file: fwrite returned a short count or 0 bytes written. Requested: " <<
                infostring.length() << ", got " << byteswritten << " bytes: " <<
                        Utility::get_errno_message(errnocopy);
    }
    fflush(filestream);

    return byteswritten;
}


