
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
#include <parse_tools.hpp>
#include <logger_tools.hpp>
#include <video_capture_globals.hpp>
#include <suspend_resume_test_thread.hpp>
#include <JsonCppUtil.hpp>
#include <commandline.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <config_tools.hpp>
#include <ConfigSingleton.hpp>
#include <vidcap_capture_thread.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_plugin_factory.hpp>
#include <json/json.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>

// CONFIGURATION
//
// Configuration of this program is affected by the initial (compiled) defaults of certain
// specific objects that can at run time be overwritten with values obtained from json configuration
// file, as well as command line parameters. That is the order of precedence:  command line options
// override json and precompiled options, json options override precompiled options, and precompiled
// options override nothing.
//
int main(int argc, const char *argv[])
{
    using namespace Video;
    using Util::Utility;

    // This vector is for lines written to the log file
    // before the logger is set up.  We will accumulate
    // logger lines in the vector until logger is set up.
    std::vector<std::string> delayedLinesForLogger;

    std::string argv0 = argv[0];
    int return_for_exit = EXIT_SUCCESS;
    VideoCapture::video_capture_plugin_factory plugin_factory;
    VideoCapture::video_plugin_base *ifptr = nullptr;

    std::string command_line_string = argv0;
    for (int ind = 1; ind < argc; ind++)
    {
        command_line_string += (std::string(" ") + Utility::string_enquote(argv[ind]));
    }

    ///////////////////////////////////////////////////////////
    // Initial processing of parsing the command line - set up the Util::CommandLine object
    // and determine whether to emit errors and/or the help feature.
    ///////////////////////////////////////////////////////////

    const Util::StringVector allowedFlags ={
            "-fn", "-pr", "-dr", "-fg", "-lg", "-fc", "-dv", "-proc-redir", "-pf", "-loginit", "-use-other-proc", "-test-suspend-resume"
    };

    Util::CommandLine cmdline(argc, argv, allowedFlags);
    if (! Video::initial_commandline_parse(cmdline, argc, argv0, std::cerr))
    {
        // If help was requested and there is no error, the process
        // will exit until after configuration help is displayed below.
        if (!cmdline.isHelp() || cmdline.isError())
        {
            std::cerr << std::endl;     // flush out std::cerr
            std::cerr << "\nNOTE:  Additional config/plugin help information not provided because of command line error.\n" << std::endl;
            return EXIT_SUCCESS;
        }
    }
    std::cerr << std::endl;         // flush out std::cerr

    ///////////////////////////////////////////////////////////
    // Set up the application (JSON-based) configuration:
    //
    // Before we do the actual parsing of the command line, the initial json/config
    // has to be done so that the command line can overwrite its values that are set
    // in the following step.
    ///////////////////////////////////////////////////////////

    std::string error_string;
    if (! Config::setup_config_singleton(error_string, delayedLinesForLogger))
    {
        std::cerr << "Config Singleton setup result: " << error_string << std::endl;
        return EXIT_FAILURE;
    }

    ///////////////////////////////////////////////////////////
    // Loading the video capture plugin happens as early as possible
    // in the program execution.  We do this right after a potential
    // emitting of the Usage message (which is followed by termination),
    // as well as the initial parsing of the json config file (so that
    // we can find the file name of the plugin to be loaded).
    ///////////////////////////////////////////////////////////
    std::string fromFactory;
    try
    {
        VideoCapture::video_plugin_base::set_base_paused(true);  // TODO: Check out this tweak more thoroughly

        std::stringstream sstrm;
        if ((ifptr = plugin_factory.create_factory(sstrm)) == nullptr)
        {
            std::cerr << "From Plugin Factory:\n" << sstrm.str() << std::endl;
            std::cerr << argv0 << ":  Could not load video capture plugin. Aborting..." << std::endl;
            return EXIT_FAILURE;
        }

        // If all goes well, the log lines reported from the plugin
        // factory will be written into the log file (the logger does
        // not yet exist at this point in time.
        fromFactory = sstrm.str();
    }
    catch (std::exception &exp)
    {
        std::cerr << argv0 << ": Got exception loading the video capture plugin. ERROR: \n" << exp.what() << ". Aborting...";
        return EXIT_FAILURE;
    }
    catch (...) {
       std::cerr << argv0 << ": General exception occurred in MAIN() loading the video capture plugin. Aborting...";
       return EXIT_FAILURE;
    }

    std::cerr << "Plugin Factory: plugin " << Utility::string_enquote(vcGlobals::str_plugin_file_name) << " was loaded and initialized successfully." << std::endl;

    ///////////////////////////////////////////////////////////
    // Parse the command line for video capture features
    ///////////////////////////////////////////////////////////

    std::string ParseOutputString;
    std::stringstream lstrm;
    if (! VidCapCommandLine::parse(lstrm, cmdline))
    {
        Video::pixel_format::displayPixelFormatConfig(std::cerr);
        std::cerr << std::endl;

        if (cmdline.isError())
        {
            std::cerr << "\nERROR in command line parsing:\n" << lstrm.str() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    ParseOutputString = std::string("\nCommand line parsing:\n") + lstrm.str();
    std::cerr << ParseOutputString << std::endl;

    // the above goes into the log file regardless of --loginit
    //                  delayedLinesForLogger.push_back(ParseOutputString);

    // if the -h flag was explicitly invoked, exit after displaying the information
    if (cmdline.isHelp())
    {
        Video::pixel_format::pixfmt_setup();
        std::cerr << "\nFrom plugin:  ";
        Video::pixel_format::displayPixelFormatConfig(std::cerr);
        std::cerr << std::endl;
        return EXIT_SUCCESS;
    }

    ///////////////////////////////////////////////////////////
    // Set up the logger
    ///////////////////////////////////////////////////////////
    setup_video_capture_logger(command_line_string, delayedLinesForLogger);
    std::shared_ptr<Log::Logger> uloggerp = Util::UtilLogger::getLoggerPtr();

    // TODO: This is going to the log regardless of --loginit:
    //                      if (vcGlobals::log_initialization_info)

    // DEBUG:  std::cerr << "\n\nFrom Plugin Factory: " << fromFactory << std::endl;

    uloggerp->debug() << "\nFrom Plugin Factory:\n" << fromFactory;

    uloggerp->debug() << "\n" << ParseOutputString;

    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    std::thread profilingthread;
    std::thread videocapturethread;
    std::thread trcc;  // gets activated only if vcGlobals::test_suspend_resume.

    bool error_termination = false;
    try
    {
        // Start the profiling thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable. See loaded plugin source - look for:
        // VideoCapture::vidcap_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        if (Video::vcGlobals::profiling_enabled)
        {
            profilingthread = std::thread(VideoCapture::video_profiler);
            uloggerp->debug() << argv0 << ":  started video profiler thread";
            profilingthread.detach();
        }

        // Start the thread which handles the queue of raw buffers that obtained from the video hardware.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler);
        queuethread.detach();

        /////////////////////////////////////////////////////////////////////
        //
        //  START THE VIDEO CAPTURE THREAD INTERFACE
        //
        /////////////////////////////////////////////////////////////////////
        uloggerp->debug() << argv0 << ":  starting the video capture thread.";

        videocapturethread = std::thread(VideoCapture::video_capture, command_line_string);
        videocapturethread.detach();
        uloggerp->debug() << argv0 << ":  kick-starting the video capture operations.";

        VideoCapture::video_plugin_base::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        ifptr->start_streaming(vcGlobals::framecount);
        uloggerp->debug() << argv0 << ":  sent start_streaming indicator to driver.";
        ifptr->set_paused(false);
        ifptr->start_profiling();
        uloggerp->debug() << argv0 << ":  kick-started the video_profiler operations.";

        // Start the test for suspend/resume (-test-suspend-resume command line flag)
        if (vcGlobals::test_suspend_resume)
        {
            // this will pause/sleep/resume/sleep a bunch of times, then signal the
            // main thread to terminate normally.
            trcc = std::thread (VideoCapture::test_raw_capture_ctl, argv0);
        }

        /////////////////////////////////////////////////////////////////////
        // At this point all threads have started and potentially are waiting
        // to be kick-started. Now we wait in the main thread...
        /////////////////////////////////////////////////////////////////////

        // wait for the video capture thread to terminate
        while (! ifptr->isterminated())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

        // CLEANUP VIDEO CAPTURE AND ITS QUEUE:

        if (ifptr->iserror_terminated())
        {
            error_termination = true;
            return_for_exit = EXIT_FAILURE;
        }

        // This signals the derived instance of the frame
        // grabber (v4l2 or opencv at this time) to terminate.
        ifptr->set_terminated(true);

        if (error_termination)
        {
            uloggerp->debug() << "main_video_capture: ERROR: Video Capture thread terminating. Cleanup and terminate.";
        }
        else
        {
            uloggerp->debug() << "main_video_capture: Video Capture thread is done. Cleanup and terminate.";
        }
    }
    catch (std::exception &exp)
    {
        error_termination = true;
        uloggerp->error() << argv0 << ": Got exception starting threads: " << exp.what() << ". Aborting...";
        return_for_exit = EXIT_FAILURE;
    } catch (...) {
       error_termination = true;
       uloggerp->error() << argv0 << ": General exception occurred in MAIN() starting the queueing and profiling threads. Aborting...";
       return_for_exit = EXIT_FAILURE;
    }

    // FINISHED:

    if (vcGlobals::test_suspend_resume)
    {
        if (!VideoCapture::suspend_resume_test::s_terminated) VideoCapture::suspend_resume_test::set_terminated(true);
        if (trcc.joinable()) trcc.join();
    }

    // Wait for the threads to finish
    if (queuethread.joinable())
    {
        if (!VideoCapture::video_capture_queue::s_terminated) VideoCapture::video_capture_queue::set_terminated(true);
        queuethread.join();
    }
    else
    {
        if (!VideoCapture::video_capture_queue::s_terminated) VideoCapture::video_capture_queue::set_terminated(true);
    }

    if (vcGlobals::profiling_enabled)
    {
        VideoCapture::vidcap_profiler::set_terminated(true);
        if(profilingthread.joinable()) profilingthread.join();
    }

    if (videocapturethread.joinable()) videocapturethread.join();

    if (error_termination)
    {
        uloggerp->error() << "ERROR:  Program terminated.";
        std::cerr << "ERROR:  Program terminated." << std::endl;
    }

    // unload the plugin
    std::stringstream dstrm;
    plugin_factory.destroy_factory(dstrm);
    uloggerp->info() << dstrm.str();

    uloggerp->info() << "Terminating the logger.";

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

#ifndef DOUBLE_FREE_ISSUE_FIXED
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: This is a hack to get around an issue with double-free/abort
    // message after the regular return which is currently commented out (see
    // below). This does not affect functionality at all. Restore the "return"
    // once this is fixed.
    //
           ::_exit(return_for_exit);
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

#else // DOUBLE_FREE_ISSUE_FIXED

    return return_for_exit;

#endif // DOUBLE_FREE_ISSUE_FIXED
}

