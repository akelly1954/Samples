#pragma once

#include <mainwindow.h>
#include <control_main.hpp>
#include <vidcap_capture_thread.hpp>
#include <condition_data.hpp>
#include <chrono>
#include <thread>
#include <iostream>

namespace NonQtUtil
{

  // This object contains some non-Qt glue to aid
  // in the management of std:: objects as well as
  // other non-Qt objects with Qt.

  class nqUtil
 {
  public:
    // This method runs in its own thread (that's why loggerp is passed as a parameter)
    static void detect_video_capture_done(std::shared_ptr<Log::Logger> loggerp, MainWindow *wp);

    // The main control thread (outside of the qt framework) takes a bit
    // to get ready. So this waits for it.  This method runs in the main() thread.
    static void initializeCapture();

    static bool isControlMainFinished;
    static std::shared_ptr<Log::Logger> loggerp;
    static const char *Argv[];
    static int Argc;
  };

} // end of namespace NonQtUtil



