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

    // Setting up the config singleton before the logger is set
    // up - we will accumulate logger lines until it is.
    std::vector<std::string> delayedLinesForLogger;

    std::string argv0 = argv[0];
    int return_for_exit = EXIT_SUCCESS;
    VideoCapture::video_capture_plugin_factory plugin_factory;
    VideoCapture::video_plugin_base *ifptr = nullptr;

    ///////////////////////////////////////////////////////////
    // Initial processing of parsing the command line - set up the Util::CommandLine object
    // and determine whether to emit errors and/or the help feature.
    ///////////////////////////////////////////////////////////

    const Util::StringVector allowedFlags ={
            "-fn", "-pr", "-fg", "-lg", "-fc", "-dv", "-proc-redir", "-pf", "-loginit", "-use-other-proc", "-test-suspend-resume"
    };

    Util::CommandLine cmdline(argc, argv, allowedFlags);
    if (! Video::initial_commandline_parse(cmdline, argc, argv0, std::cout))
    {
        std::cout << std::endl;     // flush out std::cout
        return EXIT_SUCCESS;
    }
    std::cout << std::endl;         // flush out std::cout

#if 1
    ///////////////////////////////////////////////////////////
    // Set up the application (JSON-based) configuration:
    //
    // Before we do the actual parsing of the command line, the initial json/config
    // has to be done so that the command line can overwrite its values that are set
    // in the following step.
    ///////////////////////////////////////////////////////////

    std::string ConfigOutputString;
    std::string config_results;
    if (! Config::setup_config_singleton(config_results, ConfigOutputString, delayedLinesForLogger))
    {
        std::cerr << "Config Singleton setup result: " << config_results << std::endl;
        return EXIT_FAILURE;
    }
#endif // 0

    ///////////////////////////////////////////////////////////
    // Loading the video capture plugin happens as early as possible
    // in the program execution.  We do this right after a potential
    // emitting of the Usage message followed by termination, and
    // parsing the json config file.
    ///////////////////////////////////////////////////////////
    std::string fromFactory;
    try
    {
        std::stringstream sstrm;
        if ((ifptr = plugin_factory.create_factory(sstrm)) == nullptr)
        {
            std::cerr << "From Plugin Factory:\n" << sstrm.str() << std::endl;
            std::cerr << "video_capture main:  Could not load video capture plugin. Aborting..." << std::endl;
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

    std::cout << "Plugin Factory: plugin was loaded and initialized successfully." << std::endl;

#if 0
    ///////////////////////////////////////////////////////////
    // Set up the application (JSON-based) configuration:
    //
    // Before we do the actual parsing of the command line, the initial json/config
    // has to be done so that the command line can overwrite its values that are set
    // in the following step.
    ///////////////////////////////////////////////////////////

    std::string ConfigOutputString;
    std::string config_results;
    if (! Config::setup_config_singleton(config_results, ConfigOutputString, delayedLinesForLogger))
    {
        std::cerr << "Config Singleton setup result: " << config_results << std::endl;
        return EXIT_FAILURE;
    }
#endif // 0

    ///////////////////////////////////////////////////////////
    // Parse the command line for video capture features
    ///////////////////////////////////////////////////////////

    std::string ParseOutputString;
    std::stringstream lstrm;
    if (! VidCapCommandLine::parse(lstrm, cmdline))
    {
        std::cerr << "\nERROR in command line parsing:\n" << lstrm.str() << std::endl;
        return EXIT_FAILURE;
    }

    ParseOutputString = std::string("Command line parsing:\n") + lstrm.str();
    std::cerr << ParseOutputString << std::endl;
    delayedLinesForLogger.push_back(ParseOutputString);

    ///////////////////////////////////////////////////////////
    // Set up the logger
    ///////////////////////////////////////////////////////////
    setup_video_capture_logger(ParseOutputString, ConfigOutputString, delayedLinesForLogger);
    std::shared_ptr<Log::Logger> uloggerp = Util::UtilLogger::getLoggerPtr();

    // Logger lines from the plugin factory up above
    uloggerp->debug() << "\nFrom Plugin Factory:\n" << fromFactory << "\n";
    std::cerr << "\nFrom Plugin Factory: " << fromFactory << std::endl;

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
            uloggerp->debug() << argv0 << ":  the video capture thread will kick-start the video_profiler operations.";
        }

        // Start the thread which handles the queue of raw buffers that obtained from the video hardware.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler);
        queuethread.detach();

        uloggerp->debug() << argv0 << ": kick-starting the queue operations.";
        VideoCapture::video_capture_queue::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

        /////////////////////////////////////////////////////////////////////
        //
        //  START THE VIDEO CAPTURE THREAD INTERFACE
        //
        /////////////////////////////////////////////////////////////////////
        uloggerp->debug() << argv0 << ":  starting the video capture thread.";

        videocapturethread = std::thread(VideoCapture::video_capture);
        videocapturethread.detach();
        uloggerp->debug() << argv0 << ":  kick-starting the video capture operations.";

        VideoCapture::video_plugin_base::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

        // Start the test for suspend/resume (-test-suspend-resume command line flag)
        if (vcGlobals::test_suspend_resume)
        {
            // this will pause/sleep/resume/sleep a bunch of times, then signal the
            // main thread to terminate normally.
            trcc = std::thread (VideoCapture::test_raw_capture_ctl, argv0);
        }

    #if 0
        // TODO: I think this is now going away.  Delete the lines when ready.

        // This loop waits for video capture termination (normal or otherwise).  The first second or so of
        // when video capture is initiated, the interface pointer may still be null (nullptr). Some of the
        // logged messages are commented out since they're not needed unless a runtime issue occurs.
        for (int waitforfinished = 1; ; waitforfinished++)
        {
            auto ptr = nullptr;              // TODO: ZZZ get rid of this

            if (ptr == nullptr)
            {
                if (waitforfinished == 1)
                {
                    // uloggerp->debug() << "---------------------- Started waiting, interface pointer is null";
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    continue;
                }
                else if ((waitforfinished % 7) == 0)
                {
                    ; // uloggerp->debug() << "While waiting, interface pointer is null... (counter = " << waitforfinished << ")";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                if (waitforfinished > 15)
                {
                    uloggerp->debug() << "---------------------- Interface pointer is still null. Terminating... (counter = " << waitforfinished << ")";
                    error_termination = true;
                    throw std::runtime_error ("MAIN: Video Capture thread has failed (interface pointer is null)");
                    break;
                }
                continue;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            if ((waitforfinished % 7) == 0)
            {
                ; // uloggerp->debug() << "------------------ Waiting for capture to finish... (counter = " << waitforfinished << ")";
            }
            if (ptr->isterminated())
            {
                // uloggerp->debug() << "------------------ Video capture is finished... TERMINATING...";
                break;
            }
        }
#endif // 0

        while (! ifptr->isterminated())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
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

#if 0
        uloggerp->debug() << argv0 << ":  terminating queue thread.";
        // Inform the queue handler thread that the party is over...
        VideoCapture::video_capture_queue::set_terminated(true);

        // Make sure the ring buffer gets emptied
        VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
#endif // 0
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

    if (vcGlobals::test_suspend_resume && trcc.joinable()) trcc.join();

#if 0
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
#endif // 0

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
    // std::stringstream dstrm;
    // plugin_factory.destroy_factory(dstrm);
    // TODO:  Might be ok with std::cerr...   plugin_factory.destroy_factory(std::cerr);
    // uloggerp->info() << dstrm.str();

    // uloggerp->info() << "Terminating the logger.";

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();


    ////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO: This is a hack to get around an issue with double-free/abort
    // message after the regular return which is currently commented out (see
    // below. This does not affect functionality at all. Restore the "return"
    // once this is fixed.
    //
           ::_exit(0);
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////

#if 0   // See comment above
    return return_for_exit;
#endif // 0
}

