#include <nonqt_util.hpp>

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

//  class nqUtil
// static members

const char * NonQtUtil::nqUtil::Argv[] ={
    "VideoCapturePlayer",
    "-lg",
    "DBUG",
    // "-loginit",
    // "-dr",
    "-fc",
    "0",
    // "-pr",
    nullptr
};

int NonQtUtil::nqUtil::Argc = (sizeof (Argv) / sizeof (Argv[0])) - 1;
std::shared_ptr<Log::Logger> NonQtUtil::nqUtil::loggerp = nullptr;
bool NonQtUtil::nqUtil::isControlMainFinished = false;
MainWindow *NonQtUtil::nqUtil::mwp = nullptr;

// static method
// Let the non-qt threads know that the main window is up and running
void NonQtUtil::nqUtil::setMainWindowReady(MainWindow *wp)
{
    nqUtil::mwp = wp;
}

// static method
// The loggerp param is necessary since this is a different thread to where it was defined.
void NonQtUtil::nqUtil::detect_video_capture_done(std::shared_ptr<Log::Logger> loggerp, MainWindow *wp)
{
    unsigned long sleepfor = 600;

    if (loggerp) loggerp->debug() << "detect_video_capture_done thread: Running....";
    if (!wp)
    {
        if (loggerp) loggerp->debug() << "detect_video_capture_done thread:  ERROR:  MainWindow NULL pointer.";
        return;
    }

    do
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepfor));
    } while (! nqUtil::isControlMainFinished);
    if (loggerp) loggerp->debug() << "detect_video_capture_done thread: VideoCapturePlayer ready to exit...";
    wp->CallCloseEvent();
    return;
}

// static method - runs in the main() thread
void NonQtUtil::nqUtil::initializeCapture()
{
    using NonQtUtil::nqUtil;

    unsigned long sleepfor = 400;
    int ct = 0;
    do
    {
        if ((nqUtil::loggerp = Util::UtilLogger::getLoggerPtr()) != nullptr) break;

        if (ct > 10)
        {
            std::cerr << "ERROR: Logger not initialized." << std::endl;
            return;
        }
        // std::cout << "Logger initialized after " << ct << " wait loops (" << ct*1000 << " milliseconds)." << std::endl;
        // (ct is usually 0 or 1 at this point - no biggy)

        std::this_thread::sleep_for(std::chrono::milliseconds(sleepfor));
        ct++;
    }
    while (true);

    if (nqUtil::loggerp != nullptr) nqUtil::loggerp->debug() << "Main/initializeCapture(): Initializing the capture engine to PAUSE";
    VideoCapture::video_plugin_base::set_base_paused(true);
}








