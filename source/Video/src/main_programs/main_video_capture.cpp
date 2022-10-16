
// NOTE: TODO: This file is work in progress. Look, but don't run it.

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


#ifdef TEST_RAW_CAPTURE_CTL

////////////////////////////////////////////////////////////////////////////////
// This is a debug-only short-lived thread which exercises pause/resume capture
// If used, set frame count from the command line ("-fc 0").
//
// Un-comment-out this #define if a low-level test of pause/resume/finish capture is needed.
//
// #define TEST_RAW_CAPTURE_CTL

void test_raw_capture_ctl(Log::Logger logger)
{
    logger.debug() << argv0 << ": In test_raw_capture_ctl: thread running";

    VideoCapture::vidcap_capture_base *ifptr = VideoCapture::vidcap_capture_base::get_interface_ptr();

    if (!ifptr)
    {
        std::string str = std::string("test_raw_capture_ctl thread: Could not obtain video capture iterface pointer (is null).";
        logger.warning() << str;
        throw std::runtime_error(str);
    }

    int slp = 3;
    for (int i = 1; i <= 3; i++)
    {
        ::sleep(slp);
        logger.debug() << "test_raw_capture_ctl: PAUSING CAPTURE: " << i;
        if (ifptr) ifptr->set_pause(true);

        ::sleep(slp);
        logger.debug() << "test_raw_capture_ctl: RESUMING CAPTURE: " << i;
        if (ifptr) ifptr->set_pause(false);
    }

    ::sleep(slp);
    logger.debug() << "test_raw_capture_ctl: FINISH CAPTURE REQUEST...";
    // TODO: Implement in new video_capture derived frame-grabbers
    if (ifptr) ifptr->set_terminated(true);
}

#endif // TEST_RAW_CAPTURE_CTL

// CONFIGURATION
//
// Configuration of this program is affected by the initial (compiled) defaults of certain
// specific objects that can at run time be overwritten with values obtained from json configuration
// file, as well as command line parameters.
//
// The order of precedence is simple:  If nothing else, the compiled static default values of
// objects will take effect.  Next, values of json config parameters will overwite whatever
// is in effect when they are read in.  Next, values of command line parameters overwrite whatever
// is in effect after the json values are read in.
//
// Once the json config is read in at run time, any value which comes from the json file,
// will overwrite the default value that the program was compiled with for the specific object
// being handled (lets say, the logger debug level - "DEBG", "INFO", etc).
//
// After the json config values have been written into the specifc objects they are meant
// for, command line parameters are parsed and updated into these same objects for those
// objects that are configurable from the command line.
//
// As an example:
//
//         The logger's log-level starts out in video_capture_globals.cpp defined as
//         Video::vcGlobals::loglevel, and initialized to some Log::Log::Level value, such
//         as Log::Log::Level::eDebug.
//
//         Next comes the json config file.  If the following section exists, and contains
//         a valid value, it will overwrite the object Video::vcGlobals::loglevel:
//
//         {
//             "Config": {
//                  . . . . . .
//                     "Logger": {
//                        . . . . . .
//                         "log-level":        "INFO"
//                        . . . . . .
//                     },
//                  . . . . . .
//             }
//         }
//
//         As a result of the above section, the variable Video::vcGlobals::loglevel will be
//         assigned the value Log::Log::Level::eInfo replacing compiled static value.
//
//         At the end, if the command line option "-lg NOTE" is specified, the same variable -
//         Video::vcGlobals::loglevel will finally be set to Log::Log::Level::eNotice, erasing
//         the effects of all previous values.
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
    // Parse the command line part 1 (print out the --help info asap)
    /////////////////

    std::string argv0 = argv[0];
    std::string argv1 = (argv[1] == nullptr? "-h" : argv[1]);

    // If no parameters were supplied, or help was requested:
    if (argc > 1 && (argv1 == "--help" || argv1 == "-h" || argv1 == "help"))
    {
        Video::CommandLine::Usage(std::cerr, argv0);
        std::cerr << std::endl;
        if (argc > 2)
        {
            std::cerr << "WARNING: using the --help flag negates consideration of all other flags and parameters.  Exiting...\n" << std::endl;
        }

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
        // As of this point, the root node can be accessed by reference with
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

    try {

        std::stringstream strm;
        if (! Video::updateInternalConfigsWithJsonValues(strm, Config::ConfigSingleton::instance()->JsonRoot()))
        {
            std::cerr << strm.str() << std::endl;
            std::cerr << "\nError while accessing json values read from " << Video::vcGlobals::config_file_name << "." << std::endl;
            return EXIT_FAILURE;
        }

        std::string constring = strm.str();
        std::cerr << constring << std::endl;

        // Everything in the vector will be written to the log file
        // as soon as the logger is initialized.
        delayedLinesForLogger.push_back(constring);
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

    std::stringstream lstrm;
    if (! Video::CommandLine::parse(lstrm, argc, argv))
    {
        Video::CommandLine::Usage(std::cerr, argv0);
        std::cerr << "Command line parsing:\n" << lstrm.str() << std::endl;
        return EXIT_FAILURE;
    }

    std::string lstr = std::string("Command line parsing:\n") + lstrm.str();
    std::cerr << lstr << std::endl;
    delayedLinesForLogger.push_back(lstr);

    /////////////////
    // Set up the logger
    /////////////////

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

    {
        Util::LoggerOptions logopt;

        std::stringstream ostr;
        logopt = Util::UtilLogger::getLoggerOptions();
        Util::UtilLogger::streamLoggerOptions(ostr, logopt, "after getting shared_ptr<> to Log::Logger");
        ulogger.debug() << ostr.str();
    }

    // The logger is now set up.
    ulogger.info() << "\n\nLogger setup is complete.\n";
    ulogger.info() << "    **********  Deferred output from app initialization (only displayed in DBUG mode.  **********";
    ulogger.info() << "    **********          Need to run with \"-lg DBUG\" on the command line).            **********";
    ulogger.info() << "";


    // Empty out the delayed-lines' vector...
    for(auto line : delayedLinesForLogger)
    {
        ulogger.debug() << line;
    }

    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    std::thread profilingthread;
    std::thread videocapturethread;

 #ifdef TEST_RAW_CAPTURE_CTL
    std::thread trcc;
 #endif // TEST_RAW_CAPTURE_CTL

    int return_for_exit = EXIT_SUCCESS;
    try
    {
        // Start the profiling thread if it's enabled. It wont do anything until it's kicked
        // by the condition variable (see below vidcap_profiler::s_condvar.send_ready().
        if (Video::vcGlobals::profiling_enabled)
        {
            profilingthread = std::thread(VideoCapture::video_profiler, ulogger);
            ulogger.debug() << argv0 << ":  started video profiler thread";
            profilingthread.detach();
            ulogger.debug() << argv0 << ":  kick-starting the video_profiler operations.";
            VideoCapture::vidcap_profiler::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);
        }

        // Start the thread which handles the queue of raw buffers that obtained from the video hardware.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler, ulogger, vcGlobals::output_file, vcGlobals::profiling_enabled);
        queuethread.detach();

        ulogger.debug() << argv0 << ": kick-starting the queue operations.";
        VideoCapture::video_capture_queue::s_condvar.send_ready(0, Util::condition_data<int>::NotifyEnum::All);

 #ifdef TEST_RAW_CAPTURE_CTL
         // this will pause/sleep/resume/sleep a bunch of times, then exit.
         trcc = std::thread (test_raw_capture_ctl, logger);
 #endif // TEST_RAW_CAPTURE_CTL

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
                    ; // ulogger.debug() << "---------------------- While waiting, interface pointer is null... (counter = " << waitforfinished << ")";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                if (waitforfinished > 15)
                {
                    ulogger.debug() << "---------------------- Interface pointer is still null. Terminating... (counter = " << waitforfinished << ")";
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
        ulogger.debug() << "MAIN: Video Capture thread is done. Cleanup and terminate the prgram.";

        // CLEANUP VIDEO CAPTURE AND ITS QUEUE:

        VideoCapture::vidcap_capture_base *ifptr = VideoCapture::vidcap_capture_base::get_interface_ptr();
        if (ifptr)
        {
             // This signals the derived instance of the frame
             // grabber (v4l2 or opencv at this time) to terminate.
             if (ifptr) ifptr->set_terminated(true);
        }

        ulogger.debug() << argv0 << ":  terminating queue thread.";
        // Inform the queue handler thread that the party is over...
        VideoCapture::video_capture_queue::set_terminated(true);

        // Make sure the ring buffer gets emptied
        VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
    }
    catch (std::exception &exp)
    {
        ulogger.error()
              << argv0
              << ": Got exception starting threads: "
              << exp.what()
              << ". Aborting...";
        return_for_exit = EXIT_FAILURE;
    } catch (...)
    {
       ulogger.error()
              << argv0 << ": General exception occurred in MAIN() starting the queueing and profiling threads. Aborting...";
       return_for_exit = EXIT_FAILURE;
    }

   // FINISHED:
    //      One for the display
    std::cerr << "\n"
            "To convert the output file to an mp4 file use: \n\n"
            "    $ ffmpeg -f h264 -i " << vcGlobals::output_file << " -vcodec copy " << argv0 << ".mp4\n\n"
            "Then view the mp4 file (as an example) with: \n\n"
            "    $ vlc ./" << argv0 << ".mp4\n"
            << std::endl;

    //    and one for the log file
    ulogger.info() << "";
    ulogger.info() << "To convert the output file to an mp4 file use: ";
    ulogger.info() << "";
    ulogger.info() << "    $ ffmpeg -f h264 -i " << vcGlobals::output_file << " -vcodec copy " << argv0 << ".mp4";
    ulogger.info() << "";
    ulogger.info() << "Then view the mp4 file (as an example) with: ";
    ulogger.info() << "";
    ulogger.info() << "    $ vlc ./" << argv0 << ".mp4";
    ulogger.info() << "";

    VideoCapture::vidcap_profiler::set_terminated(true);

    #ifdef TEST_RAW_CAPTURE_CTL
    if (trcc.joinable()) trcc.join();
    #endif // TEST_RAW_CAPTURE_CTL

    // Wait for the threads to finish
    if (queuethread.joinable()) queuethread.join();
    if (vcGlobals::profiling_enabled && profilingthread.joinable()) profilingthread.join();
    if (videocapturethread.joinable()) videocapturethread.join();

    ulogger.info() << "Terminating the logger.";

    // Terminate the Log Manager (destroy the Output objects)
    Log::Manager::terminate();

    return return_for_exit;
}

