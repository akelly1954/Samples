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

  nqUtil::isControlMainFinished = false;

  std::thread control_main_thread;
  std::thread detect_video_capture_done_thread;

  control_main_thread = std::thread(control_main, nqUtil::Argc, nqUtil::Argv);
  control_main_thread.detach();

  // this waits for the video capture thread to start running and initialize.
  nqUtil::initializeCapture();

  // The significance of ifptr being non-null at this point is that
  // the capture engine has been started after the video capture plugin
  // has been loaded.  If it winds up being null, something very very bad
  // has happened (examine stdout output, as well as the log file to find
  // out what happened.
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  QApplication a(argc, argv);
  MainWindow w(nqUtil::loggerp);

  detect_video_capture_done_thread = std::thread(nqUtil::detect_video_capture_done, nqUtil::loggerp, &w);
  detect_video_capture_done_thread.detach();

  if (nqUtil::loggerp != nullptr) nqUtil::loggerp->debug() << "Main: Initializing the capture engine to PAUSE";
  if (ifptr) ifptr->set_paused(true);

  w.show();

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
