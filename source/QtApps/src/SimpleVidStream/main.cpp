
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

#include <mainwindow.h>
#include <control_main.hpp>
#include <vidcap_capture_thread.hpp>
#include <condition_data.hpp>
#include <nonqt_util.hpp>
#include <chrono>
#include <thread>
#include <iostream>
#include <QApplication>

int main(int argc, char *argv[])
{
  using NonQtUtil::nqUtil;

  QApplication a(argc, argv);

  nqUtil::isControlMainFinished = false;

  std::thread control_main_thread;
  std::thread detect_video_capture_done_thread;

  // control_main runs in its own thread.
  control_main_thread = std::thread(control_main, nqUtil::Argc, nqUtil::Argv);
  control_main_thread.detach();

  // this waits for the video capture thread to start running and initialize.
  // The function runs in this (main) thread.
  nqUtil::initializeCapture();

  // The significance of ifptr being non-null at this point is that
  // the capture engine has been started after the video capture plugin
  // has been loaded.  If it winds up being null, something very very bad
  // has happened (examine stdout output, as well as the log file to find
  // out what happened.
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  if (ifptr == nullptr)
  {
    std::cerr << "Main: Fatal ERROR: Could not obtain valid pointer to plugin.  Exiting...." << std::endl;

    // See comment at the end of main()
    ::_Exit(0);
    // TODO: Should be - return EXIT_FAILURE;
  }

  // QApplication a(argc, argv);
  MainWindow w(nqUtil::loggerp);
  w.show();

  detect_video_capture_done_thread = std::thread(nqUtil::detect_video_capture_done, nqUtil::loggerp, &w);
  detect_video_capture_done_thread.detach();

  /* TODO: If the "return ret" below is uncommented, the "int ret" declaration needs to be as well.
   * int ret = */

  a.exec();

  if (control_main_thread.joinable())  control_main_thread.join();
  if (detect_video_capture_done_thread.joinable())  detect_video_capture_done_thread.join();

  // TODO: _Exit() is used instead of return to avoid "double-free"
  //       exception after the return (unwinding objects).  BUG.

  ::_Exit(0);
  // return ret;
}
