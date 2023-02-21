#include <mainwindow.h>
#include <control_main.hpp>
#include <vidcap_capture_thread.hpp>
#include <condition_data.hpp>
#include <chrono>
#include <thread>
#include <iostream>

#include <QApplication>

///////////////////////////////////////////////////////////////////////
// TODO: For now, these two static objects are set at compile time.
const char *Argv[] = {
  "SimpleVidStream",
  "-lg",
  "DBUG",
  // "-loginit",
  "-dr",
  "-fc",
  "800",
  "-pr",
  nullptr
};
int Argc = (sizeof (Argv) / sizeof (Argv[0])) - 1;
///////////////////////////////////////////////////////////////////////

std::shared_ptr<Log::Logger> loggerp = nullptr;

// The main control thread (outside of the qt framework) takes a bit
// to get ready. So this waits for it.
void initializeCapture()
{
  unsigned long sleepfor = 400;
  int ct = 0;
  do
  {
    if ((loggerp = Util::UtilLogger::getLoggerPtr()) != nullptr) break;

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
}

bool isControlMainFinished = false;

void detect_video_capture_done(std::shared_ptr<Log::Logger> loggerp, MainWindow *wp)
{
  unsigned long sleepfor = 600;

  if (loggerp) loggerp->debug() << "detect_video_capture_thread: Running....";
  if (!wp)
  {
    if (loggerp) loggerp->debug() << "detect_video_capture_done:  ERROR:  MainWindow NULL pointer.";
    return;
  }

  do
  {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleepfor));
  } while (! isControlMainFinished);
  if (loggerp) loggerp->debug() << "Done with detect_video_capture_thread";
  wp->CallCloseEvent();
  return;
}

int main(int argc, char *argv[])
{
  isControlMainFinished = false;

  std::thread control_main_thread;
  std::thread detect_video_capture_done_thread;

  control_main_thread = std::thread(control_main, Argc, Argv);
  control_main_thread.detach();
  initializeCapture();

  // The significance of ifptr being non-null at this point is that
  // the capture engine has been started after the video capture plugin
  // has been loaded.  If it winds up being null, something very very bad
  // has happened.
  VideoCapture::video_plugin_base *ifptr = VideoCapture::video_plugin_base::get_interface_pointer();

  QApplication a(argc, argv);
  MainWindow w(loggerp);

  detect_video_capture_done_thread = std::thread(detect_video_capture_done, loggerp, &w);
  detect_video_capture_done_thread.detach();

  if (loggerp != nullptr) loggerp->debug() << "Main: Initializing the capture engine to PAUSE";
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
