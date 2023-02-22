#include <nonqt_util.hpp>

//  class nqUtil
// static members

const char * NonQtUtil::nqUtil::Argv[] ={
      "SimpleVidStream",
      "-lg",
      "DBUG",
      // "-loginit",
      "-dr",
      "-fc",
      "300",
      "-pr",
      nullptr
};

int NonQtUtil::nqUtil::Argc = (sizeof (Argv) / sizeof (Argv[0])) - 1;
std::shared_ptr<Log::Logger> NonQtUtil::nqUtil::loggerp = nullptr;
bool NonQtUtil::nqUtil::isControlMainFinished = false;

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
  if (loggerp) loggerp->debug() << "detect_video_capture_done thread: SimpleVidStream ready to exit...";
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
}








