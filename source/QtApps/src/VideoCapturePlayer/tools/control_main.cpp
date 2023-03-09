
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

#include <control_main.hpp>
#include <nonqt_util.hpp>
#include <video_capture_commandline.hpp>
#include <parse_tools.hpp>
#include <logger_tools.hpp>
#include <vidstream_profiler_thread.hpp>
#include <video_capture_globals.hpp>
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

// NOTE:
//
// This object mixes Qt objects with external std:: based objects (including std::thread), as well
// as other external libraries (.so, .a, etc).  For a period of time this object will be a mess -
// a hodge podge of slightly compatible technologies.
//
// As time goes on, some objects will be transitioned to Qt (QThreads, etc), while other objects
// (Log::Logger, some condition variables) will stay as they are.  Take care.
//

// This will delay return from control_main until everything is cleaned up properly.
Util::condition_data<int> s_condvar(0);

int control_main(int argc, const char *argv[])
{
    using namespace Video;
    using NonQtUtil::nqUtil;
    using Util::Utility;

    // TODO: Do something about this...  should become a video capture method which
    //       initializes main-specific globals close to the beginning of most/all main()'s.
    vcGlobals::logChannelName =                   "video_capture_player";
    vcGlobals::logFilelName =                     Video::vcGlobals::logChannelName + "_log.txt";
    vcGlobals::output_file =                      Video::vcGlobals::logChannelName + ".data"; // Name of file intended for the video frames
    vcGlobals::runtime_config_output_file =       Video::vcGlobals::logChannelName + "_runtime_config.txt";
    vcGlobals::loglevel =                         Log::Log::Level::eDebug;
    vcGlobals::log_level =                        Log::Log::toString(Video::vcGlobals::loglevel);
    vcGlobals::config_file_name =                 Video::vcGlobals::logChannelName + ".json";
    VideoCapture::video_plugin_base::set_base_paused(true);
    Video::vcGlobals::profile_logprint_enabled =  false;     // We don't want two profilers writing to the log

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
        // std::cerr << "argv[" << ind << "] is " << argv[ind]  << std::endl;
        command_line_string += " ";
        command_line_string += Utility::string_enquote(argv[ind]);
    }

    ///////////////////////////////////////////////////////////
    // Initial processing of parsing the command line - set up the Util::CommandLine object
    // and determine whether to emit errors and/or the help feature.
    ///////////////////////////////////////////////////////////

    const Util::StringVector allowedFlags ={
            "-fn", "-pr", "-dr", "-fg", "-lg", "-fc", "-dv", "-proc-redir", "-pf", "-loginit", "-use-other-proc"
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
    // std::cerr << ParseOutputString << std::endl;
    delayedLinesForLogger.push_back(ParseOutputString);

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

    if (vcGlobals::log_initialization_info)
    {
        // Logger lines from the plugin factory up above
        uloggerp->debug() << "\nFrom Plugin Factory:\n" << fromFactory;
    }
    // DEBUG:  std::cerr << "\nFrom Plugin Factory: " << fromFactory << std::endl;

    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    std::thread profilingthread;
    std::thread videocapturethread;
    //    std::thread vidstreamprofilerthread;
    ProfilingController pctl;

    bool error_termination = false;
    try
    {
        // Start the profiing thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable. See loaded plugin source - look for:
        // VideoCapture::vidcap_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        if (Video::vcGlobals::profiling_enabled)
        {
            VideoCapture::vidcap_profiler::set_terminated(false);
            profilingthread = std::thread(VideoCapture::video_profiler);
            uloggerp->debug() << argv0 << ":  started video profiler thread";
            profilingthread.detach();
            uloggerp->debug() << argv0 << ":  the video capture thread will kick-start the video_profiler operations.";

            uloggerp->debug() << argv0 << ":  Starting the vidstream profiler thread operations.";
            pctl.operateProfilingStats();
            // VideoCapture::vidstream_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
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
    nqUtil::isControlMainFinished = true;

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

        // VideoCapture::vidstream_profiler::set_terminated(true);
        // if(vidstreamprofilerthread.joinable()) vidstreamprofilerthread.join();
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

    // Free up the waiting Qt MainWindow thread if it's waiting.
    s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

    // Terminate the Log Manager (destroy the Output objects)
    uloggerp->info() << "Terminating the logger.";

    // Give the rest of this thread a chance to truly be finished.
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    Log::Manager::terminate();

    // Qt (MainWindow) gets its toes tied in a knot if the return value is not zero...
    return 0;
}

// Called from the MainWindow thread - it will wait until we finish cleaning up this thread.
void control_main_wait_for_ready()
{
  std::shared_ptr<Log::Logger> loggerp = Util::UtilLogger::getLoggerPtr();
  if (loggerp)  loggerp->debug() << "control_main_wait_for_ready: WAITING";
  s_condvar.wait_for_ready();
  if (loggerp)  loggerp->debug() << "control_main_wait_for_ready: RELEASED";
}
