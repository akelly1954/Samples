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

#include <video_capture_raw_queue.hpp>
#include <v4l2_raw_capture.h>
#include <Utility.hpp>
#include <commandline.hpp>
#include <NtwkUtil.hpp>
#include <NtwkFixedArray.hpp>
#include <LoggerCpp/LoggerCpp.h>
#include <stdio.h>
#include <thread>

std::string logChannelName = "v4l2_raw_capture";
std::string logFilelName = logChannelName + "_log.txt";

std::string output_file = logChannelName + ".yuv";  // Name of file intended for the video frames

Log::Log::Level loglevel = Log::Log::Level::eDebug;
std::string default_log_level = "debug";
std::string log_level = default_log_level;

size_t framecount = 200;
std::string default_frame_count(std::to_string(framecount));
std::string frame_count(default_frame_count);

bool parse(std::ostream &strm, int argc, char *argv[]);


//////////////////////////////////////////////////////////////////////////////////
// Interface to C language functions section
//////////////////////////////////////////////////////////////////////////////////

// Set up logging facility (for the C code) roughly equivalent to std::cerr...
// Filthy code but I have to deal with C.
Log::Logger *global_logger = NULL;

#ifdef __cplusplus
    extern "C" void v4l2capture_logger(const char *logmessage);
#else
    void v4l2capture_logger(const char *logmessage);
#endif // __cplusplus

void v4l2capture_logger(const char *logmessage)
{
    if (global_logger == NULL)
    {
        fprintf (stderr, "%s\n", logmessage);
    }
    else
    {
        global_logger->debug() << logmessage;
    }
}

void (*logger_function)(const char *logmessage) = v4l2capture_logger;

// Setup of the callback function (from the C code).
// Called back for every buffer that becomes available.
#ifdef __cplusplus
    extern "C" void v4l2capture_callback(void *p, size_t size);
#else
    void v4l2capture_callback(void *p, size_t size);
#endif // __cplusplus

void v4l2capture_callback(void *p, size_t bsize)
{
    if (p != NULL)
    {
        size_t bytesleft = bsize;
        uint8_t *startdata = static_cast<uint8_t*>(p);
        size_t nextsize = 0;

        while (bytesleft > 0)
        {
            nextsize = bytesleft > EnetUtil::NtwkUtilBufferSize? EnetUtil::NtwkUtilBufferSize : bytesleft;

            std::shared_ptr<EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>> sp =
                        EnetUtil::fixed_size_array<uint8_t,EnetUtil::NtwkUtilBufferSize>::create(startdata, nextsize);

            startdata += nextsize;
            bytesleft -= nextsize;
            assert (sp->num_valid_elements() == nextsize);

            // Add the shared_ptr to the queue
            VideoCapture::video_capture_queue::s_ringbuf.put(sp, VideoCapture::video_capture_queue::s_condvar);
        }
    }
}

void (*callback_function)(void *, size_t) = v4l2capture_callback;

//////////////////////////////////////////////////////////////////////////////////
// Command line parsing section
//////////////////////////////////////////////////////////////////////////////////

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
            << "              [ -lg log-level ]         Can be one of: {\"debug\", \"info\", \"notice\", \"warning\",\n"
            << "                                        \"error\", \"critical\"}. (The default is \"notice\")\n"
            << "\n"
            << std::endl;
}

/////////////////////////////////////////////////////////////////////////////////
// Main program
//////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
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
    Util::Utility::initializeLogManager(configList, loglevel, logFilelName, Util::Utility::disableConsole, Util::Utility::enableLogFile);
    Util::Utility::configureLogManager( configList, logChannelName );
    Log::Logger logger(logChannelName.c_str());

    // Setup of the logger callback function (from the C code).
    global_logger = &logger;

    std::cerr << "Log level is: " << log_level << std::endl;
    if (VideoCapture::video_capture_queue::s_write_frames_to_file)
    {
        std::cerr << "Ouput file: " << output_file << std::endl;
    }

    std::cerr << "Frame count is: " << framecount << std::endl;
    if (framecount > 100)
    {
        std::cerr << "\nWARNING: a high frame count can create a huge output file.\n" << std::endl;
    }
    /////////////////
    // Finally, get to work
    /////////////////

    std::thread queuethread;
    int ret = 0;
    try
    {
        // Start the thread which handles the queue of raw buffers that the callback
        // function handles.
        queuethread = std::thread(VideoCapture::raw_buffer_queue_handler, logger, output_file);

        // This is the C based code close to the hardware
        // The options desired here are the "-o" flag and "-c".
        char *fakeargv[] =
            {
                const_cast<char *>(logChannelName.c_str()),
                const_cast<char *>("-o"),
                const_cast<char *>("-c"),
                const_cast<char *>(frame_count.c_str()),
                NULL
            };
        int fakeargc = sizeof(*fakeargv)/sizeof(fakeargv[0])-1;
        ret = ::v4l2_raw_capture_main(4, fakeargv, callback_function, logger_function);

        // Inform the queue handler thread that the party is over...
        VideoCapture::video_capture_queue::set_terminated(true);

       // Make sure the ring buffer gets emptied
        VideoCapture::video_capture_queue::s_condvar.flush(0, Util::condition_data<int>::NotifyEnum::All);
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

    // Wait for the queue thread to finish
    if (queuethread.joinable()) queuethread.join();

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
            strm << "Writing frames to file will remain the default: ";
                if( VideoCapture::video_capture_queue::s_write_frames_to_file )
                {
                    strm << "Write-frames-to-file, file name is: \"" << output_file << "\"" << std::endl;
                }
                else
                {
                    strm << "Do-not-write-frames-to-file." << std::endl;
                }
            break;
        case Util::ParameterStatus::FlagPresentParameterPresent:
            VideoCapture::video_capture_queue::set_write_frames_to_file(true);
            strm << "Turning on write-frames-to-file, using: \"" << output_file << "\"." << std::endl;
            break;
        case Util::ParameterStatus::FlagProvidedWithEmptyParameter:
            VideoCapture::video_capture_queue::set_write_frames_to_file(true);
            strm << "Turning on write-frames-to-file, using the default: \"" <<
                    output_file << "\"." << std::endl;
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
            assert (argc == -670);   // Bug encountered. Will cause abnormal termination
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

    // Assign the frame count
    framecount = strtoul(frame_count.c_str(), NULL, 10);

    return true;
}








