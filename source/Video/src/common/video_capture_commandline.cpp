
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

#include <video_capture_commandline.hpp>
#include <video_capture_globals.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <commandline.hpp>

void Video::CommandLine::Usage(std::ostream &strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help (or -h or help)" << "\n";
    strm << "Or:       " << command
            << "\n"
            << "              [ -fn [ file-name ] ]     Turns on the write-frames-to-file functionality.  The file-name \n"
            << "                                        parameter is the file which will be created to hold image frames.\n"
            << "                                        If it exists, the file will be truncated. If the file name is omitted,\n"
            << "                                        the default name \"" << Video::vcGlobals::output_file << "\" will be used.\n"
            << "              [ -pr [ timeslice_ms]     Enable profiler stats. If specified, the optional parameter is the number of\n"
            << "                                        milliseconds between profiler snapshots. The default is " << Video::vcGlobals::profile_timeslice_ms << " milliseconds.\n"
            << "              [ -lg log-level ]         Can be one of: {\"DBUG\", \"INFO\", \"NOTE\", \"WARN\",\n"
            << "                                        \"EROR\", \"CRIT\"}. (The default is " << Video::vcGlobals::default_log_level << ")\n"
            << "              [ -fg [ video-grabber ]]  The video frame grabber to be used. Can be one of {\"v4l2\", \"opencv\"}. (The default\n"
            << "                                        grabber is \"v4l2\").\n"
            << "              [ -fc frame-count ]       Number of frames to grab from the hardware (default is " << Video::vcGlobals::framecount << ")\n"
            << "              [ -dv video-device ]      The /dev entry for the video camera. (The default value is " << Video::vcGlobals::str_dev_name << ")\n"
            << "              [ -pf pixel-format ]      The pixel format requested from the video driver, which can be \"h264\" or \"yuyv\".\n"
            << "                                        These are:\n"
            << "                                                  " << Video::vcGlobals::pixel_formats_strings[Video::pxl_formats::h264] << "\n"
            << "                                                  " << Video::vcGlobals::pixel_formats_strings[Video::pxl_formats::yuyv] << "\n"
            << "                                                  Please see /usr/include/linux/videodev2.h for more information\n"
            << "                                        (The default pixel-format value is \"h264\").\n";
}

bool Video::CommandLine::parse(std::ostream &strm, int argc, const char *argv[])
{
    using namespace Util;

    const std::vector<std::string> allowedFlags ={ "-fn", "-pr", "-fg", "-lg", "-fc", "-dv", "-pf" };
    const std::map<std::string, std::string> cmdmap = getCLMap(argc, argv, allowedFlags);

    // this flag (-fn) and an existing readable regular file name are MANDATORY
    switch(getArg(cmdmap, "-fn", Video::vcGlobals::output_file))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // This means write-to-file may or may not be enabled:  Use the default
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            Video::vcGlobals::write_frames_to_file = true;
            // strm << "Turning on write-frames-to-file, using: \"" << Video::vcGlobals::output_file << "\"." << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::write_frames_to_file = true;
            // strm << "Turning on write-frames-to-file, using the default: \"" <<
            //         Video::vcGlobals::output_file << "\"." << "\n";
            break;
        default:
            assert (argc == -668);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    write-frames-to-file is set to enabled, file name is " << Video::vcGlobals::output_file << "\n";


    // assignment to vcGlobals happens after everything else has been assigned below.
    int fcount_value = Video::vcGlobals::framecount;
    switch(getArg(cmdmap, "-fc", fcount_value))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debug:
            //      strm << "    -fc flag not provided. Using default " << fcount_value << "\n";
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debug:
            //      strm << "    -fc flag provided. Using " << fcount_value << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "    WARNING: \"-fc\" flag with no parameter: using default " << fcount_value << "\n";
            break;
        default:
            assert (argc == -669);   // Bug encountered. Will cause abnormal termination
    }
    // Value for frame count is checked at the very end.

    int default_timeslice = Video::vcGlobals::profile_timeslice_ms;
    switch(getArg(cmdmap, "-pr", default_timeslice))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // Use the default Video::vcGlobals::profiling_enabled
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debug:
            //      strm << "    -fc flag provided. Using " << fcount_value << "\n";
            Video::vcGlobals::profiling_enabled = true;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::profiling_enabled = true;
            break;
        default:
            assert (argc == -670);   // Bug encountered. Will cause abnormal termination
    }
    if (default_timeslice <= 0)
    {
        strm << "ERROR: Profiling timeslice (" << default_timeslice << ") has to be positive.\n";
        return false;
    }
    Video::vcGlobals::profile_timeslice_ms = default_timeslice;
    strm << "    Profiling is set to " << (Video::vcGlobals::profiling_enabled? "enabled" : "disabled")
         << ", timeslice = " << Video::vcGlobals::profile_timeslice_ms << ".\n";


    // this flag (-fg) is not mandatory but needs a frame-grabber name if it is specified
    switch(getArg(cmdmap, "-fg", Video::vcGlobals::video_grabber_name))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // This means use the default
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // strm << "Setting frame-grabber to  \"" << Video::vcGlobals::video_grabber_name << "\"." << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-fg\" flag has no parameter. Needs video grabber name." << "\n";
            return false;
        default:
            assert (argc == -671);   // Bug encountered. Will cause abnormal termination
    }


    switch(getArg(cmdmap, "-lg", Video::vcGlobals::log_level))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:
            // strm << "-lg flag not provided. Using default " << Video::vcGlobals::log_level << "\n";
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:
            // strm << "-lg flag provided. Using " << Video::vcGlobals::log_level << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-lg\" flag is missing its parameter." << "\n";
            return false;
        default:
            assert (argc == -672);   // Bug encountered. Will cause abnormal termination
    }


    switch(getArg(cmdmap, "-dv", Video::vcGlobals::str_dev_name))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:
            // strm << "-dv flag not provided. Using default " << Video::vcGlobals::str_dev_name << "\n";
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:
            // strm << "-dv flag provided. Using " << Video::vcGlobals::str_dev_name << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-dv\" flag is missing its parameter." << "\n";
            return false;
        default:
            assert (argc == -673);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    Video frame grabber device name is set to " << Video::vcGlobals::str_dev_name << "\n";


    std::string pixelfmt = (Video::vcGlobals::pixel_format == Video::pxl_formats::h264? "h264": "yuyv");
    switch(getArg(cmdmap, "-pf", pixelfmt))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:
            // strm << "    -pf flag not provided. Using default " << pixelfmt << "\n";
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:
            // strm << "    -pf flag provided. Using " << pixelfmt << "\n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-pf\" flag is missing its parameter." << "\n";
            return false;
        default:
            assert (argc == -674);   // Bug encountered. Will cause abnormal termination
    }

    /////////////////
    // Check out the specified pixel format name
    /////////////////
    if (pixelfmt == "h264")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::h264;
    }
    else if (pixelfmt == "yuyv")
    {
        Video::vcGlobals::pixel_format = Video::pxl_formats::yuyv;
    }
    else
    {
        strm << "ERROR: pixel format is invalid: " << pixelfmt << "\n";
        return false;
    }
    strm << "    Video pixel format is set to: " << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_format] << "\n";

    /////////////////
    // Check out the specified video frame grabber name
    /////////////////

    if (Video::vcGlobals::video_grabber_name != "v4l2" && Video::vcGlobals::video_grabber_name != "opencv")
    {
        strm << "\nERROR: Invalid video frame grabber name. Grabber names can be any one of:  {\"v4l2\", \"opencv\"}." << "\n";
        return false;
    }
    strm << "    Video frame grabber name is set to " << Video::vcGlobals::video_grabber_name << "\n";

    /////////////////
    // Check out specified log level
    /////////////////
    int level = UtilLogger::stringToEnumLoglevel(Video::vcGlobals::log_level);
    if (level >= 0)
    {
        Video::vcGlobals::loglevel = static_cast<Log::Log::Level>(level);
    }
    else
    {
        strm << "\nERROR: Invalid log_level. Values can be any one of: {\"DBUG\", \"INFO\", \"NOTE\", \"WARN\", \"EROR\", \"CRIT\"}." << "\n";
        return false;
    }
    strm << "    Log level is set to " << Video::vcGlobals::loglevel << " = " << Video::vcGlobals::log_level << "\n";

    // Assign the frame count (only after the command line parameters were applied)
    // Frame count set to 0 means stream non-stop.
    if (fcount_value < 0)
    {
        strm << "\nERROR: frane count (" << fcount_value << "): The number has to be positive or 0.\n";
        return false;
    }

    Video::vcGlobals::framecount = fcount_value;
    Video::vcGlobals::str_frame_count = std::to_string(Video::vcGlobals::framecount);
    strm << "    Frame count is set to " << Video::vcGlobals::framecount << "(int) = " << Video::vcGlobals::str_frame_count << "(string)\n";

    return true;
}
