//
// This is the C++ main fronting for the C main (called here v4l2_raw_capture_main()).
//

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

#include <v4l2_interface.hpp>
#include <video_capture_raw_queue.hpp>
#include <video_capture_profiler.hpp>
#include <v4l2_raw_capture.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <commandline.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <unistd.h>
#include <stdio.h>
#include <thread>

//////////////////////////////////////////////////////////////////////////////////
//
// If the H264 encoding is used in this program, it will create an mp4 file out of it,
// playable by vlc or whatever:
//
//             ffmpeg -f h264 -i v4l2_raw_capture.data -vcodec copy v4l2_raw_capture.mp4
//
//////////////////////////////////////////////////////////////////////////////////


// See the end of this source file for this function
bool parse(std::ostream &strm, int argc, char *argv[]);

////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, set frame count from the command line ("-fc 0").
// Un-comment-out this #define if a low-level test of pause/resume/finish capture is needed.
//
// #define TEST_RAW_CAPTURE_CTL

#ifdef TEST_RAW_CAPTURE_CTL

void test_raw_capture_ctl(Log::Logger logger)
{
    logger.debug() << "In test_raw_capture_ctl: thread running";

    for (int i = 1; i <= 3; i++)
    {
        ::sleep(3);
        logger.debug() << "test_raw_capture_ctl: PAUSING CAPTURE: " << i;
        ::set_v4l2capture_pause(true);

        ::sleep(3);
        logger.debug() << "test_raw_capture_ctl: RESUMING CAPTURE: " << i;
        ::set_v4l2capture_pause(false);
    }

    ::sleep(3);
    logger.debug() << "test_raw_capture_ctl: FINISH CAPTURE REQUEST...";
    ::set_v4l2capture_finished();
}

#endif // TEST_RAW_CAPTURE_CTL


void Usage(std::ostream &strm, std::string command)
{
    strm << "\nUsage:    " << command << " --help (or -h or help)" << std::endl;
    strm << "Or:       " << command
            << "\n"
            << "              [ -fn [ file-name ] ]     Turns on the write-frames-to-file functionality.  The file-name \n"
            << "                                        parameter is the file which will be created to hold image frames.\n"
            << "                                        If it exists, the file will be truncated. If the file name is omitted,\n"
            << "                                        the default name \"" << output_file << "\" will be used.\n"
            << "              [ -fc frame-count ]       Number of frames to grab from the hardware (default is " << framecount << ")\n"
            << "              [ -pr ]                   Enable profiler stats\n"
            << "              [ -lg log-level ]         Can be one of: {\"debug\", \"info\", \"notice\", \"warning\",\n"
            << "                                        \"error\", \"critical\"}. (The default is \"notice\")\n"
            << "\n"
            << std::endl;
}


int main(int argc, char *argv[])
{
    using namespace VideoCapture;

    /////////////////
    // Parse the command line
    /////////////////

    std::string argv0 = const_cast<const char*>(argv[0]);

    // If no parameters were supplied, or help was requested:
    if (argc > 1 && (std::string(const_cast<const char*>(argv[1])) == "--help"
                    || std::string(const_cast<const char*>(argv[1])) == "-h"
                    || std::string(const_cast<const char*>(argv[1])) == "help")
            )
    {
        Usage(std::cerr, argv0);
        if (argc > 2)
        {
            std::cerr << "WARNING: using the --help flag negates consideration of all other flags and parameters.  Exiting...\n" << std::endl;
        }

        return 0;
    }

    if (! parse(std::cerr, argc, argv))
    {
        Usage(std::cerr, argv0);
        return 1;
    }

    /////////////////
    // Set up the logger
    /////////////////

    Log::Config::Vector configList;
    Util::MainLogger::initializeLogManager(configList, loglevel, logFilelName, Util::MainLogger::disableConsole, Util::MainLogger::enableLogFile);
    Util::MainLogger::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName.c_str());

    std::cerr << "Log level is: " << log_level << std::endl;
    if (video_capture_queue::s_write_frames_to_file)
    {
        std::cerr << "Ouput file: " << output_file << std::endl;
    }

    std::cerr << (profiling_enabled? std::string("Profiling: enabled"): std::string("Profiling: disabled")) << std::endl;

    std::cerr << "Frame count is: " << framecount << std::endl;
    if (framecount > 500)
    {
        std::cerr << "\nWARNING: a high frame count can create a huge output file.\n" << std::endl;
    }

    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    std::thread profilingthread;

    // this can be turned on/off in ...Samples/source/Video/CMakeLists.txt
#ifdef TEST_RAW_CAPTURE_CTL
    std::thread trcc;
#endif // TEST_RAW_CAPTURE_CTL

    int ret = 0;
    try
    {
        // Start the profiling thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable (see below video_capture_profiler::s_condvar.send_ready().
        if (profiling_enabled)
        {
            profilingthread = std::thread(video_profiler, logger);
            profilingthread.detach();
            video_capture_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        }

        // Start the thread which handles the queue of raw buffers that the callback
        // function handles.
        queuethread = std::thread(raw_buffer_queue_handler, logger, output_file, profiling_enabled);
        queuethread.detach();

// this can be turned on/off in ...Samples/source/Video/CMakeLists.txt
#ifdef TEST_RAW_CAPTURE_CTL
        // this will pause/sleep/resume/sleep a bunch of times, then exit.
        trcc = std::thread (test_raw_capture_ctl, logger);
#endif // TEST_RAW_CAPTURE_CTL

        // This is glue for the C based code close to the hardware
        ::v4l2capture_set_callback_functions(
                            pause_function,
                            finished_function,
                            terminate_function,
                            callback_function,
                            logger_function,
                            logger_stream_function
                        );

        // The options desired here are the "-o" flag and "-c".
        char *fakeargv[] =
            {
                const_cast<char *>(logChannelName.c_str()),
                const_cast<char *>("-o"),
                const_cast<char *>("-F"),  // force H264 1920x1080 format
                // OR: const_cast<char *>("-f"),  // force YUYV 640x480 format
                // OR: nothing - default to whatever the video is set to (YUYV 640x320?)
                const_cast<char *>("-c"),
                const_cast<char *>(frame_count.c_str()),
                NULL
            };
        int fakeargc = sizeof(fakeargv)/sizeof(fakeargv[0])-1;  // the -1 is for the NULL at the end

        ret = ::v4l2_raw_capture_main(fakeargc, fakeargv);

        // Inform the queue handler thread that the party is over...
        video_capture_queue::set_terminated(true);

       // Make sure the ring buffer gets emptied
        video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    }
    catch (std::exception &exp)
    {
        logger.error()
                << "Got exception in main() starting the raw_buffer_queue_handler thread "
                << exp.what();
        ret = 1;
    } catch (...)
    {
        logger.error()
                << "General exception occurred in MAIN() the raw_buffer_queue_handler thread ";
        ret = 1;
    }

    // FINISHED:
    //      One for the display
    std::cerr << "\n"
    		"To convert the output file to an mp4 file use: \n\n"
    		"    $ ffmpeg -f h264 -i " << output_file << " -vcodec copy v4l2_raw_capture.mp4\n\n"
    		"Then view the mp4 file (as an example) with: \n\n"
    		"    $ vlc ./v4l2_raw_capture.mp4\n"
			<< std::endl;

    //    and one for the log file
    logger.info() << "";
    logger.info() << "To convert the output file to an mp4 file use: ";
	logger.info() << "";
	logger.info() << "    $ ffmpeg -f h264 -i " << output_file << " -vcodec copy v4l2_raw_capture.mp4";
	logger.info() << "";
	logger.info() << "Then view the mp4 file (as an example) with: ";
	logger.info() << "";
	logger.info() << "    $ vlc ./v4l2_raw_capture.mp4";
	logger.info() << "";

    // CLEANUP
    ::set_v4l2capture_finished();
    video_capture_profiler::set_terminated(true);

// this can be turned on/off in ...Samples/source/Video/CMakeLists.txt
#ifdef TEST_RAW_CAPTURE_CTL
    if (trcc.joinable()) trcc.join();
#endif // TEST_RAW_CAPTURE_CTL

    // Wait for the queue thread to finish
    if (queuethread.joinable()) queuethread.join();
    if (profiling_enabled && profilingthread.joinable()) profilingthread.join();

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return ret;
}

bool parse(std::ostream &strm, int argc, char *argv[])
{
    using namespace Util;
    const std::map<std::string, std::string> cmdmap = getCLMap(argc, argv);

    // this flag (-fn) and an existing readable regular file name are MANDATORY
    switch(getArg(cmdmap, "-fn", output_file))
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

    switch(getArg(cmdmap, "-fc", frame_count))
    {
        case Util::ParameterStatus::FlagNotProvided:
            // for debugging:  strm << "-fc flag not provided. Using default " << frame_count << std::endl;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            // for debugging:  strm << "-fc flag provided. Using " << frame_count << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            strm << "ERROR: \"-fc\" flag is missing its parameter." << std::endl;
            return false;
        default:
            assert (argc == -669);   // Bug encountered. Will cause abnormal termination
    }

    switch(getArg(cmdmap, "-pr", frame_count))
    {
        case Util::ParameterStatus::FlagNotProvided:
            profiling_enabled = false;
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            strm << "ERROR: \"-pr\" flag has a parameter where no parameters are allowed." << std::endl;
            return false;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            profiling_enabled = true;
            break;
        default:
            assert (argc == -670);   // Bug encountered. Will cause abnormal termination
    }

    switch(getArg(cmdmap, "-lg", log_level))
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

    if (log_level == "debug") loglevel = Log::Log::eDebug;
    else if (log_level == "info") loglevel = Log::Log::eInfo;
    else if (log_level == "notice") loglevel = Log::Log::eNotice;
    else if (log_level == "warning") loglevel = Log::Log::eWarning;
    else if (log_level == "error") loglevel = Log::Log::eError;
    else if (log_level == "critical") loglevel = Log::Log::eCritic;
    else
    {
        strm << "ERROR: Incorrect use of the \"-lg\" flag." << std::endl;
        return false;
    }

    // Assign the frame count (only after the command line parameters were applied)
    // Frame count set to 0 means stream non-stop.
    framecount = strtoul(frame_count.c_str(), NULL, 10);

    return true;
}


