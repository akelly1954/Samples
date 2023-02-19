#include <mainwindow.h>
#include <control_main.hpp>
#include <thread>

#include <QApplication>

// TODO: For now, these two static objects are set at compile time.
const char *Argv[] = {
  "SimpleVidStream",
  "-lg",
  "DBUG",
  "-loginit",
  "-dr",
  "-fc",
  "300",
  nullptr
};
int Argc = (sizeof (Argv) / sizeof (Argv[0])) - 1;

int main(int argc, char *argv[])
{
  std::thread control_main_thread;

  control_main_thread = std::thread(control_main, Argc, Argv);
  control_main_thread.detach();

  QApplication a(argc, argv);
  MainWindow w;
  w.show();
  int ret = a.exec();

  if (control_main_thread.joinable())  control_main_thread.join();
  // TODO: _Exit() is used instead of return to avoid "double-free"
  //       exception after the return (unwinding objects).  BUG.

  ::_Exit(ret);

  //return ret;
  // return a.exec();
}
