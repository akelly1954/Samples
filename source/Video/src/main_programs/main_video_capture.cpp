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
#include <video_capture_globals.hpp>
#include <suspend_resume_test_thread.hpp>
#include <JsonCppUtil.hpp>
#include <commandline.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <config_tools.hpp>
#include <ConfigSingleton.hpp>
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

// static definitions

Util::LoggerOptions setLocalLoggerOptions()
{
    Util::LoggerOptions localopt = {
            // loglevel (in LoggerOptions)
            Video::vcGlobals::loglevel,

            // log_level (in LoggerOptions)
            Video::vcGlobals::log_level,

            // logChannelName (in LoggerOptions)
            Video::vcGlobals::logChannelName,

            // logFileName (in LoggerOptions)
            Video::vcGlobals::logFilelName,

            // useConsole  (in LoggerOptions)
            Util::MainLogger::disableConsole,

            // useLogFile (in LoggerOptions)
            Util::MainLogger::enableLogFile
    };
    return localopt;
}

int main(int argc, const char *argv[])
{
    using namespace Video;

    // Setting up the config singleton before the logger is set
    // up - we will accumulate logger lines until it is.
    std::vector<std::string> delayedLinesForLogger;

    std::string argv0 = argv[0];
    int return_for_exit = EXIT_SUCCESS;

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
        return 0;
    }
    std::cout << std::endl;         // flush out std::cout

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

    /////////////////
    // Parse the command line part 2
    /////////////////

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

    /////////////////
    // Set up the logger
    /////////////////

    // Remove an existing log file before instantiating the logger:
    // This can only be done after command line parsing is finished.
    ::unlink(Video::vcGlobals::logFilelName.c_str());

    // This picks the values from Video::vcGlobals (which was modified
    // by the json file and then the command line.
    Util::LoggerOptions localopt = setLocalLoggerOptions();

    // set the values in localopt as the values in ulogger.
    Util::UtilLogger::setLoggerOptions(localopt);

    //////////////////////////////////////////////////////////////
    // Initialize the UtilLogger object
    //////////////////////////////////////////////////////////////
    Util::UtilLogger::create(localopt);

    //////////////////////////////////////////////////////////////
    // Reference to THE logger object
    //////////////////////////////////////////////////////////////
    Log::Logger& ulogger = *(Util::UtilLogger::getLoggerPtr());

    //////////////////////////////////////////////////////////////
    // Start Logging
    //////////////////////////////////////////////////////////////

    ulogger.info() << "START OF NEW VIDEO CAPTURE RUN";

    {
        Util::LoggerOptions logopt;
        std::stringstream ostr;
        logopt = Util::UtilLogger::getLoggerOptions();

        Util::UtilLogger::streamLoggerOptions(ostr, logopt, "after getting shared_ptr<> to Log::Logger");
        ulogger.debug() << ostr.str();
    }

    // The logger is now set up.
    ulogger.info() << "\n\nLogger setup is complete.\n";
    ulogger.info() << "";

    if (vcGlobals::log_initialization_info)     // -loginit flag
    {
        ulogger.info() << "\n\n    ******  Deferred output from app initialization:  ******\n";

        // Empty out the delayed-lines' vector...
        for(auto line : delayedLinesForLogger)
        {
            ulogger.info() << "DELAYED: " << line;
        }
    }
    else
    {
        // -loginit flag was not specified: Capture the last few lines into the log file
        ulogger.info() << "The last few lines of deferred output from app initialization are shown here.   ******";
        ulogger.info() << "For the full set of deferred lines, use the -loginit flag on the command line.  ******";

        ulogger.info() << "DELAYED: .  .  .  . . . .\n" << ConfigOutputString << "\n";
        ulogger.info() << "DELAYED: .  .  .  . . . .\n\n" << ParseOutputString;
    }

    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    std::thread profilingthread;
    std::thread videocapturethread;  // TODO: XXX
    std::thread trcc;  // gets activated only if vcGlobals::test_suspend_resume.

    bool error_termination = false;
    try
    {
        // Start the profiling thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable (see below vidcap_profiler::s_condvar.send_ready().
        if (Video::vcGlobals::profiling_enabled)
        {
            // TODO: XXX       profilingthread = std::thread(VideoCapture::video_profiler, ulogger);
            ulogger.debug() << argv0 << ":  started video profiler thread";
            profilingthread.detach();
            ulogger.debug() << argv0 << ":  the video capture thread will kick-start the video_profiler operations.";
        }

        // Start the thread which handles the queue of raw buffers that obtained from the video hardware.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler, ulogger);
        queuethread.detach();

        ulogger.debug() << argv0 << ": kick-starting the queue operations.";
        VideoCapture::video_capture_queue::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

        /////////////////////////////////////////////////////////////////////
        //
        //  START THE VIDEO CAPTURE THREAD INTERFACE
        //
        /////////////////////////////////////////////////////////////////////
        ulogger.debug() << argv0 << ":  starting the video capture thread.";

        // TODO: XXX               videocapturethread = std::thread(VideoCapture::video_capture, ulogger);
        // The plugin factory runs in its own thread, loads the plugin and initializes it.
        // TODO: XXX               videocapturethread = std::thread(VideoCapture::video_capture_factory, ulogger);
        videocapturethread = std::thread(video_capture_factory, ulogger);

        //  TODO: XXX                videocapturethread.detach();

        ulogger.debug() << argv0 << ":  kick-starting the video capture operations.";

        // TODO: ZZZ             VideoCapture::vidcap_capture_base::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

        // Start the test for suspend/resume (-test-suspend-resume command line flag)
        if (vcGlobals::test_suspend_resume)
        {
            // this will pause/sleep/resume/sleep a bunch of times, then signal the
            // main thread to terminate normally.
            trcc = std::thread (VideoCapture::test_raw_capture_ctl, ulogger, argv0);
        }

#if 0
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
                    // ulogger.debug() << "---------------------- Started waiting, interface pointer is null";
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));
                    continue;
                }
                else if ((waitforfinished % 7) == 0)
                {
                    ; // ulogger.debug() << "While waiting, interface pointer is null... (counter = " << waitforfinished << ")";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                if (waitforfinished > 15)
                {
                    ulogger.debug() << "---------------------- Interface pointer is still null. Terminating... (counter = " << waitforfinished << ")";
                    error_termination = true;
                    throw std::runtime_error ("MAIN: Video Capture thread has failed (interface pointer is null)");
                    break;
                }
                continue;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            if ((waitforfinished % 7) == 0)
            {
                ; // ulogger.debug() << "------------------ Waiting for capture to finish... (counter = " << waitforfinished << ")";
            }
            if (ptr->isterminated())
            {
                // ulogger.debug() << "------------------ Video capture is finished... TERMINATING...";
                break;
            }
        }
        // CLEANUP VIDEO CAPTURE AND ITS QUEUE:

        VideoCapture::vidcap_capture_base *ifptr = VideoCapture::vidcap_capture_base::get_interface_ptr();
        if (ifptr)
        {
            if (ifptr->iserror_terminated())
            {
                error_termination = true;
                return_for_exit = EXIT_FAILURE;
            }

            // This signals the derived instance of the frame
            // grabber (v4l2 or opencv at this time) to terminate.
            ifptr->set_terminated(true);
        }
#endif // 0

        if (error_termination)
        {
            ulogger.debug() << "MAIN: ERROR: Video Capture thread terminating. Cleanup and terminate.";
        }
        else
        {
            ulogger.debug() << "MAIN: Video Capture thread is done. Cleanup and terminate.";
        }

        ulogger.debug() << argv0 << ":  terminating queue thread.";
        // Inform the queue handler thread that the party is over...
        VideoCapture::video_capture_queue::set_terminated(true);

        // Make sure the ring buffer gets emptied
        VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    }
    catch (std::exception &exp)
    {
        error_termination = true;
        ulogger.error()
              << argv0
              << ": Got exception starting threads: "
              << exp.what()
              << ". Aborting...";
        return_for_exit = EXIT_FAILURE;
    } catch (...)
    {
       error_termination = true;
       ulogger.error()
              << argv0 << ": General exception occurred in MAIN() starting the queueing and profiling threads. Aborting...";
       return_for_exit = EXIT_FAILURE;
    }

    // FINISHED:

    if (vcGlobals::test_suspend_resume && trcc.joinable()) trcc.join();

    // Wait for the threads to finish
    if (queuethread.joinable()) queuethread.join();
    if (vcGlobals::profiling_enabled && profilingthread.joinable()) profilingthread.join();
    if (videocapturethread.joinable()) videocapturethread.join();

    if (error_termination)
    {
        ulogger.error() << "ERROR:  Program terminated.";
        std::cerr << "ERROR:  Program terminated." << std::endl;
    }

    ulogger.info() << "Terminating the logger.";

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return return_for_exit;
}

