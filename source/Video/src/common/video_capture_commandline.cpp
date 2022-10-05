
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
#include <video_capture_raw_queue.hpp>
#include <commandline.hpp>

void Video::CommandLine::Usage(std::ostream &strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help (or -h or help)" << std::endl;
    strm << "Or:       " << command
            << "\n"
            << "              [ -fn [ file-name ] ]     Turns on the write-frames-to-file functionality.  The file-name \n"
            << "                                        parameter is the file which will be created to hold image frames.\n"
            << "                                        If it exists, the file will be truncated. If the file name is omitted,\n"
            << "                                        the default name \"" << Video::vcGlobals::output_file << "\" will be used.\n"
            << "              [ -fc frame-count ]       Number of frames to grab from the hardware (default is " << Video::vcGlobals::framecount << ")\n"
            << "              [ -pr ]                   Enable profiler stats\n"
            << "              [ -lg log-level ]         Can be one of: {\"debug\", \"info\", \"notice\", \"warning\",\n"
            << "                                        \"error\", \"critical\"}. (The default is " << Video::vcGlobals::default_log_level << ")\n";
}

bool Video::CommandLine::parse(std::ostream &strm, int argc, const char *argv[])
{
    using namespace Util;
    const std::map<std::string, std::string> cmdmap = getCLMap(argc, argv);

    // this flag (-fn) and an existing readable regular file name are MANDATORY
    switch(getArg(cmdmap, "-fn", Video::vcGlobals::output_file))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // This means write-to-file may or may not be enabled:  Use the default
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            VideoCapture::video_capture_queue::set_write_frames_to_file(true);
            // strm << "Turning on write-frames-to-file, using: \"" << output_file << "\"." << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            VideoCapture::video_capture_queue::set_write_frames_to_file(true);
            // strm << "Turning on write-frames-to-file, using the default: \"" <<
            //         output_file << "\"." << std::endl;
            break;
        default:
            assert (argc == -668);   // Bug encountered. Will cause abnormal termination
    }

    switch(getArg(cmdmap, "-fc", Video::vcGlobals::str_frame_count))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-fc flag not provided. Using default " << str_frame_count << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-fc flag provided. Using " << str_frame_count << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-fc\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (argc == -669);   // Bug encountered. Will cause abnormal termination
    }

    switch(getArg(cmdmap, "-pr", Video::vcGlobals::profiling_enabled))
    {
        case Util::ParameterStatus::FlagNotProvided:
            Video::vcGlobals::profiling_enabled = false;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            strm << "ERROR: \"-pr\" flag has a parameter where no parameters are allowed." << std::endl;
            return false;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            Video::vcGlobals::profiling_enabled = true;
            break;
        default:
            assert (argc == -670);   // Bug encountered. Will cause abnormal termination
    }

    switch(getArg(cmdmap, "-lg", Video::vcGlobals::log_level))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-lg flag not provided. Using default " << log_level << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-lg flag provided. Using " << log_level << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-lg\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (argc == -671);   // Bug encountered. Will cause abnormal termination
    }

    /////////////////
    // Check out specified log level
    /////////////////

    if (Video::vcGlobals::log_level == "debug") Video::vcGlobals::loglevel = Log::Log::eDebug;
    else if (Video::vcGlobals::log_level == "info") Video::vcGlobals::loglevel = Log::Log::eInfo;
    else if (Video::vcGlobals::log_level == "notice") Video::vcGlobals::loglevel = Log::Log::eNotice;
    else if (Video::vcGlobals::log_level == "warning") Video::vcGlobals::loglevel = Log::Log::eWarning;
    else if (Video::vcGlobals::log_level == "error") Video::vcGlobals::loglevel = Log::Log::eError;
    else if (Video::vcGlobals::log_level == "critical") Video::vcGlobals::loglevel = Log::Log::eCritic;
    else
    {
        strm << "ERROR: Incorrect use of the \"-lg\" flag." << std::endl;
        return false;
    }

    // Assign the frame count (only after the command line parameters were applied)
    // Frame count set to 0 means stream non-stop.
    Video::vcGlobals::framecount = strtoul(Video::vcGlobals::str_frame_count.c_str(), NULL, 10);

    return true;
}