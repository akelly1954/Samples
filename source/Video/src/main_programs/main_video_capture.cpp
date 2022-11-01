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
#include <JsonCppUtil.hpp>
#include <commandline.hpp>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <ConfigSingleton.hpp>
#include <vidcap_profiler_thread.hpp>
#include <vidcap_raw_queue_thread.hpp>
#include <vidcap_capture_thread.hpp>
#include <json/json.h>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <vector>

////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, the frame count is automatically set to 0 (like "-fc 0").
//
void test_raw_capture_ctl(Log::Logger logger, std::string argv0)
{
    int sleep_seconds = 3;
    int i = 0;

    ::sleep(sleep_seconds);
    logger.debug() << argv0 << ": In test_raw_capture_ctl: thread running";

    VideoCapture::vidcap_capture_base *ifptr = VideoCapture::vidcap_capture_base::get_interface_ptr();

    if (!ifptr)
    {
        std::string str("test_raw_capture_ctl thread: Could not obtain video capture interface pointer (is null).");
        logger.warning() << str;
        throw std::runtime_error(str);
    }

    int slp = sleep_seconds;
    for (i = 1; i <= 10 && !ifptr->isterminated(); i++)
    {
        logger.debug() << "test_raw_capture_ctl: RESUMED/RUNNING: waiting " << slp << " seconds before pausing. Pass # " << i;
        ::sleep(slp);  if (ifptr->isterminated()) { break; }
        logger.debug() << "test_raw_capture_ctl: PAUSING CAPTURE: " << i;
        if (ifptr) ifptr->set_paused(true);

        logger.debug() << "test_raw_capture_ctl: PAUSED: waiting " << slp << " seconds before resuming. Pass # " << i;
        ::sleep(slp);  if (ifptr->isterminated()) { break; }
        logger.debug() << "test_raw_capture_ctl: RESUMING CAPTURE: " << i;
        if (ifptr) ifptr->set_paused(false);
    }

    if (!ifptr->isterminated())
    {
        ::sleep(slp);
    }
    else
    {
        logger.debug() << "test_raw_capture_ctl: other threads terminated before test finished. TERMINATING AFTER " << i << " PASSES...";
        return;
    }

    logger.debug() << "test_raw_capture_ctl: FINISH CAPTURE REQUEST...";
    if (ifptr) ifptr->set_terminated(true);
}

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

    /////////////////
    // Parse the command line part 1 (print out the --help or parse-error info asap)
    /////////////////

    std::string argv0 = argv[0];
    int return_for_exit = EXIT_SUCCESS;

    const Util::StringVector allowedFlags ={
            "-fn", "-pr", "-fg", "-lg", "-fc", "-dv", "-proc-redir", "-pf", "-loginit", "-use-other-proc", "-test-suspend-resume"
    };

    Util::CommandLine cmdline(argc, argv, allowedFlags);

    if(cmdline.isError())
    {
        std::cout << "\n" << argv0 << ": " << cmdline.getErrorString() << "\n" << std::endl;
        VidCapCommandLine::Usage(std::cout, argv0);
        std::cout << std::endl;
        return EXIT_FAILURE;
    }

    if(cmdline.isHelp())
    {
        if (argc > 2)
        {
            std::cout << "\nWARNING: using the --help flag cancels all other flags and parameters.  Exiting...\n" << std::endl;
        }
        VidCapCommandLine::Usage(std::cout, argv0);
        std::cout << std::endl;
        return EXIT_SUCCESS;
    }

    /////////////////
    // Set up the application configuration:
    //
    // Before we do the actual parsing of the command line, the initial json/config
    // has to be done so that the command line can overwrite its values in the following step.
    /////////////////
    std::stringstream loggerStream;

    try {
        /////////////////
        // Set up config
        /////////////////

        // The assignment is unnecessary - It's here to silence g++ warnings
        // (about discarding the return value)...
        auto thesp = Config::ConfigSingleton::create(Video::vcGlobals::config_file_name, loggerStream);

        // At this point the json root node has been set up - after parsing, checking syntax, etc.
        // If ANY errors are encountered along the way, they will be catch()ed below and the
        // program aborted.
        //
        // The root node can be accessed by reference with
        //
        //              Json::Value& ConfigSingleton::instance()->JsonRoot();
        //

    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception while trying to create config singleton: \n    " << e.what() << std::endl;
        std::cerr << "Previously logged info: " << loggerStream.str() << std::endl;
        return EXIT_FAILURE;
    }

    // We will log this as soon as the logger is configured and operational.
    std::string parse_output = std::string("\n\nParsed JSON nodes:") + loggerStream.str();

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(parse_output);

    std::stringstream configstrm;
    configstrm << "\n\nParsed JSON file " << Video::vcGlobals::config_file_name << " successfully.  Contents: \n\n"
                      << Config::ConfigSingleton::instance()->JsonRoot() << "\n";

    // Everything in the vector will be written to the log file
    // as soon as the logger is initialized.
    delayedLinesForLogger.push_back(configstrm.str());
    std::string ConfigOutputString;
    try {

        std::stringstream strm;
        if (! Video::updateInternalConfigsWithJsonValues(strm, Config::ConfigSingleton::instance()->JsonRoot()))
        {
            std::cerr << strm.str() << std::endl;
            std::cerr << "\nError while accessing json values read from " << Video::vcGlobals::config_file_name << "." << std::endl;
            return EXIT_FAILURE;
        }

        ConfigOutputString = strm.str();

        // TODO:  This just clutters up the screen.  Leave it in the log file:
        // std::cerr << ConfigOutputString << std::endl;

        // Everything in the vector will be written to the log file
        // as soon as the logger is initialized.
        delayedLinesForLogger.push_back(ConfigOutputString);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error:  Exception caught while accessing json values read from "
                  << Video::vcGlobals::config_file_name << ": "
                  << e.what() << std::endl;
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
    std::thread videocapturethread;
    std::thread trcc;

    bool error_termination = false;
    try
    {
        // Start the profiling thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable (see below vidcap_profiler::s_condvar.send_ready().
        if (Video::vcGlobals::profiling_enabled)
        {
            profilingthread = std::thread(VideoCapture::video_profiler, ulogger);
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
        videocapturethread = std::thread(VideoCapture::video_capture, ulogger);
        videocapturethread.detach();

        ulogger.debug() << argv0 << ":  kick-starting the video capture operations.";
        VideoCapture::vidcap_capture_base::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

        // this will pause/sleep/resume/sleep a bunch of times, then signal the
        // main thread to terminate normally.
        trcc = std::thread (test_raw_capture_ctl, ulogger, argv0);

        // This loop waits for video capture termination (normal or otherwise).  The first second or so of
        // when video capture is initiated, the interface pointer may still be null (nullptr). Some of the
        // logged messages are commented out since they're not needed unless a runtime issue occurs.
        for (int waitforfinished = 1; ; waitforfinished++)
        {
            auto ptr = VideoCapture::vidcap_capture_base::get_interface_ptr();
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

    if (trcc.joinable()) trcc.join();

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

