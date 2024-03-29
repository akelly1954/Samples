
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
#include <Utility.hpp>

void Video::VidCapCommandLine::Usage(std::ostream &strm, std::string command)
{
    using namespace Video;
    using Util::Utility;

    strm << "\nUsage:    " << command << " --help (or -h or help)" << "\n";
    strm << "Or:       " << command
            << "\n"
            << "              [ -fn [ file-name ]]      -Turns on the \"write-to-file\" functionality (see JSON file).  The file-name \n"
            << "                                        parameter is the file which will be created to hold image frames. If it exists, \n"
            << "                                        the file will be truncated. If the file name is omitted, the default name used \n"
            << "                                        is the runtime value of \"output-file\" in the Json config file. (By default, the \n"
            << "                                        \"write-to-file\" capability is turned off in favor of the \"write-to-process\" \n"
            << "                                        member in the JSON config file).\n"
            << "              [ -pr [ timeslice_ms ]]   -Enable profiler stats. If specified, the optional parameter is the number of \n"
            << "                                        milliseconds between profiler snapshots. (The default is the runtime value of \n"
            << "                                        \"profile-timeslice-ms\" in the Json config file).\n"
            << "              [ -lg log-level ]         -Can be one of: {\"DBUG\", \"INFO\", \"NOTE\", \"WARN\", \"EROR\", \"CRIT\"}. \n"
            << "                                        (The default is the runtime value of \"log-level\" in the Json config file).\n"
            << "              [ -loginit ]              -(no parameters) This flag enables the logging of initialization info.\n"
            << "              [ -dr [ file-name ] ]     -Write out detailed runtime configuration to the parameter \"file-name\" instead of \n"
            << "                                        the log file. The default file-name is " << Utility::string_enquote(vcGlobals::runtime_config_output_file) << ". \n"
            << "                                        The information provided is a snapshot of the runtime configuration after all json options \n"
            << "                                        as well as command-line options have been set, and the plugin has been loaded. \n"
            << "              [ -fg video-grabber ]     -The video frame grabber to be used. Can be one of {\"v4l2\", \"opencv\"}. (The \n"
            << "                                        default grabber is the runtime value of \"preferred-interface\" in the Json config file).\n"
            << "              [ -fc frame-count ]       -Number of frames to grab from the hardware. (The default is the runtime value of \n"
            << "                                        \"frame-count\" in the Json config file).\n"
            << "              [ -dv video-device ]      -The /dev entry for the video camera. (The default value is the runtime value of\n"
            << "                                        \"device-name\" in the Json config file in the section named for the video-grabber used.\n"
            << "              [ -proc-redir [ file ]]   -If the \"write-to-process\" member of the JSON config file is set to 1 (enabled), \n"
            << "                                        the process which is started and streamed to (typically ffmpeg) has its \"standard \n"
            << "                                        error\" still open to the controlling display (terminal).  To get rid of the extra \n"
            << "                                        output on the screen, std::cerr, (stderr, fd 2, etc) can be redirected to a regular \n"
            << "                                        file or to \"/dev/null\" as needed by using this flag and a filename. If the \"file\" \n"
            << "                                        parameter is not specified, the standard error output will go to the screen. \n"
            << "                                        (By default, the flag is enabled, and the filename used is \"/dev/null\").\n"
            << "              [ -use-other-proc ]       -(no parameters) This flag directs the program to use the \"other\" entry (within the \n"
            << "                                        \"pixel-format\" section of \"preferred-interface\" of the \"frame-capture\" section of \n"
            << "                                        the JSON configuration file) as the command string to use for popen() instead of the \n"
            << "                                        default command string indicated by the name of the pixel format chosen.\n"
            << "              [ -pf pixel-format ]      -The pixel format requested from the video driver, which can be \"h264\" or \"yuyv\".\n"
            << "                                        These are:\n"
            << "                                                  " << vcGlobals::pixel_formats_strings[pxl_formats::h264] << "\n"
            << "                                                  " << vcGlobals::pixel_formats_strings[pxl_formats::yuyv] << "\n"
            << "                                        Please see /usr/include/linux/videodev2.h for more information\n"
            << "                                        (The default pixel-format value is the runtime value of \"preferred-pixel-format\"\n"
            << "                                        in the Json config file in the section named for the video-grabber used).\n"
            << "              [ -test-suspend-resume ]  -(no parameters) The program will run a special thread that first sets the frame-count\n"
            << "                                        to 0 (regardless of command-line or JSON settings), and then it allows main() to run. It \n"
            << "                                        then interrupts the flow of video frames every few seconds with a \"pause\" request, waits\n"
            << "                                        a few seconds and then \"resume\"s. This goes on for 30 or 40 seconds, and then it terminates\n"
            << "                                        the program. The effects on the program and data flow can be seen in the log file.\n";
}

bool Video::VidCapCommandLine::parse(std::ostream &strm, Util::CommandLine& cmdline)
{
    using namespace Util;

    if (cmdline.isError() || cmdline.isHelp()) return false;

    int fail_int = 101010;      // this is just for the assert()s

    // this flag (-loginit) enables logging of initial delayed log output
    std::string tstr;;
    switch(cmdline.get_template_arg("-loginit", tstr))
    {
        case Util::ParameterStatus::FlagNotProvided:
            vcGlobals::log_initialization_info = false;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            strm << "ERROR: \"-loginit\" has a parameter where none is allowed." << "\n";
            return false;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            vcGlobals::log_initialization_info = true;
            break;
        default:
            assert (fail_int == -667);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    Logging of initialization lines is set to " << (vcGlobals::log_initialization_info? "true" : "false") << ".\n";

    std::string redirect_file = Video::vcGlobals::redir_filename;
    switch(cmdline.get_template_arg("-proc-redir", Video::vcGlobals::redir_filename))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // Do not change anything
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            Video::vcGlobals::proc_redir = true;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            // This means do not redirect process output
            Video::vcGlobals::proc_redir = false;
            break;
        default:
            assert (fail_int == -668);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    redirect stderr to file is set to "
         << (Video::vcGlobals::proc_redir == false? "false": "true") << "\n";

    if (Video::vcGlobals::proc_redir)
    {
        strm << "    Stderr output from the process streamed to, will be redirected to " << Video::vcGlobals::redir_filename << "\n";
    }
    else
    {
        strm << "    Stderr output from the process streamed to, will be not be redirected. It may quite possibly go to the screen. \n";
    }

    switch(cmdline.get_template_arg("-fn", Video::vcGlobals::output_file))
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
            assert (fail_int == -669);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    write-frames-to-file is set to " << Utility::stringify_bool(Video::vcGlobals::write_frames_to_file)
         << ", file name is " << Utility::string_enquote(Video::vcGlobals::output_file) << "\n";

    switch(cmdline.get_template_arg("-dr", Video::vcGlobals::runtime_config_output_file))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // This means display_runtime_config may or may not be enabled:  Use the default
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            Video::vcGlobals::display_runtime_config = true;
            // strm << "Turning on display_runtime_config, using: " << Utility::string_enquote(Video::vcGlobals::runtime_config_output_file) << ". \n";
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::display_runtime_config = true;
            // strm << "Turning on display_runtime_config, using the default: " <<
            //         Utility::string_enquote(Video::vcGlobals::runtime_config_output_file) << ". \n";
            break;
        default:
            assert (fail_int == -1669);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    display_runtime_config is set to " << Utility::stringify_bool(Video::vcGlobals::display_runtime_config)
         << ", output file name is " << Utility::string_enquote(Video::vcGlobals::runtime_config_output_file) << "\n";

    // assignment to vcGlobals happens after everything else has been assigned below.
    int fcount_value = Video::vcGlobals::framecount;
    switch(cmdline.get_template_arg("-fc", fcount_value))
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
            assert (fail_int == -670);   // Bug encountered. Will cause abnormal termination
    }
    // Value for frame count is checked at the very end.

    int default_timeslice = Video::vcGlobals::profile_timeslice_ms;
    switch(cmdline.get_template_arg("-pr", default_timeslice))
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
            assert (fail_int == -671);   // Bug encountered. Will cause abnormal termination
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
    switch(cmdline.get_template_arg("-fg", Video::vcGlobals::video_grabber_name))
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
            assert (fail_int == -672);   // Bug encountered. Will cause abnormal termination
    }


    switch(cmdline.get_template_arg("-lg", Video::vcGlobals::log_level))
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
            assert (fail_int == -673);   // Bug encountered. Will cause abnormal termination
    }


    switch(cmdline.get_template_arg("-dv", Video::vcGlobals::str_dev_name))
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
            assert (fail_int == -674);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    Video frame grabber device name is set to " << Video::vcGlobals::str_dev_name << "\n";


    std::string pixelfmt = (Video::vcGlobals::pixel_fmt == Video::pxl_formats::h264? "h264": "yuyv");
    switch(cmdline.get_template_arg("-pf", pixelfmt))
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
            assert (fail_int == -675);   // Bug encountered. Will cause abnormal termination
    }

    /////////////////
    // Check out the specified pixel format name
    /////////////////
    if (pixelfmt == "h264")
    {
        Video::vcGlobals::pixel_fmt = Video::pxl_formats::h264;
    }
    else if (pixelfmt == "yuyv")
    {
        Video::vcGlobals::pixel_fmt = Video::pxl_formats::yuyv;
    }
    else
    {
        strm << "ERROR: pixel format is invalid: " << pixelfmt << "\n";
        return false;
    }
    strm << "    Video pixel format is set to: " << Video::vcGlobals::pixel_formats_strings[Video::vcGlobals::pixel_fmt] << "\n";


    int useotherproc = static_cast<int>(Video::vcGlobals::use_other_proc);
    switch(cmdline.get_template_arg("-use-other-proc", useotherproc))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // Do not change anything
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            strm << "ERROR: \"-use-other-proc\" has a parameter where none is allowed." << "\n";
            return false;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::use_other_proc = true;
            break;
        default:
            assert (fail_int == -676);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    use_other_proc is set to "
         << (Video::vcGlobals::use_other_proc == false? "false": "true") << "\n";


    int testsuspendresume = static_cast<int>(Video::vcGlobals::test_suspend_resume);
    switch(cmdline.get_template_arg("-test-suspend-resume", testsuspendresume))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // Do not change anything
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            strm << "ERROR: \"-test-suspend-resume\" has a parameter where none is allowed." << "\n";
            return false;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::test_suspend_resume = true;
            break;
        default:
            assert (fail_int == -677);   // Bug encountered. Will cause abnormal termination
    }
    strm << "    test_suspend_resume is set to "
         << (Video::vcGlobals::test_suspend_resume == false? "false": "true") << "\n";
    // See more on this below

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
        strm << "\nERROR: frame count (" << fcount_value << "): The number has to be positive or 0.\n";
        return false;
    }

    // Are we testing suspend/resume?
    if (Video::vcGlobals::test_suspend_resume)
    {
        if (fcount_value > 0 && fcount_value < 300)
        {
            fcount_value = 300;
            strm << "\nWARNING: Adjusting frame count to " << fcount_value << " for suspend/resume testing\n";
            std::cerr << "        WARNING: Adjusting frame count to " << fcount_value << " for suspend/resume testing" << std::endl;
        }
        Video::vcGlobals::set_framecount(fcount_value);
        strm << "\nWARNING: Setting frame count to " << Video::vcGlobals::str_frame_count << " for testing (trcc thread).\n" ;
        std::cerr << "        WARNING: Setting frame count to " << Video::vcGlobals::str_frame_count << " for testing (trcc thread)." << std::endl;
    }
    else
    {
        Video::vcGlobals::set_framecount(fcount_value);
        strm << "    Frame count is set to " << Video::vcGlobals::framecount << "(int) = " << Video::vcGlobals::str_frame_count << "(string)\n";
        // std::cerr << "    Frame count is set to " << Video::vcGlobals::framecount << "(int) = " << Video::vcGlobals::str_frame_count << "(string)" << std::endl;
    }

    if (fcount_value == 0)
    {
        strm << "\nWARNING: Frame count is set to " << fcount_value << " - continuous streaming.\n";
        std::cerr << "        WARNING: Frame count is set to " << fcount_value << " - continuous streaming." << std::endl;
    }
    return true;
}
