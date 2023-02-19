#include <mainwindow.h>
#include <control_main.hpp>
#include <chrono>
#include <thread>
#include <iostream>

#include <QApplication>

// TODO: For now, these two static objects are set at compile time.
const char *Argv[] = {
  "SimpleVidStream",
  "-lg",
  "DBUG",
  "-loginit",
  "-dr",
  "-fc",
  "800",
  "-pr",
  nullptr
};
int Argc = (sizeof (Argv) / sizeof (Argv[0])) - 1;

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
      std::cerr << "ERROR: Logger not initialized. Exiting..." << std::endl;
      return;
    }
    // std::cout << "Logger initialized after " << ct << " wait loops (" << ct*1000 << " milliseconds)." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(sleepfor));
    ct++;
  }
  while (true);
}

int main(int argc, char *argv[])
{
  std::thread control_main_thread;

  control_main_thread = std::thread(control_main, Argc, Argv);
  control_main_thread.detach();
  initializeCapture();

  QApplication a(argc, argv);
  MainWindow w(loggerp);
  w.show();
  int ret = a.exec();

  if (control_main_thread.joinable())  control_main_thread.join();
  // TODO: _Exit() is used instead of return to avoid "double-free"
  //       exception after the return (unwinding objects).  BUG.

  ::_Exit(0);

  // return ret;
}
