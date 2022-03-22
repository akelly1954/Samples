#include "main_text_finder_window.h"
#include <QApplication>
#include <Utility.hpp>
#include <MainLogger.hpp>
#include <string>
#include <memory>
#include <mutex>

std::string string_log_channel_name = "text_finder";
Log::Log::Level loglevel = Log::Log::Level::eDebug;
Util::MainLogger::ConsoleOutput useConsole = Util::MainLogger::ConsoleOutput::enableConsole;
Util::MainLogger::UseLogFile useLogFile = Util::MainLogger::UseLogFile::disableLogFile;


int main(int argc, char *argv[])
{
    /////////////////
    // Set up the logger
    /////////////////

    Util::MainLogger::initialize(string_log_channel_name, loglevel, useConsole, useLogFile);
    Log::Logger logger(string_log_channel_name.c_str());

    logger.debug() << "Calling QApplication";

    QApplication a(argc, argv);
    TextFinder w;
    w.show();
    return a.exec();
}
