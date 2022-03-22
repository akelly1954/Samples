#include "main_text_finder_window.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TextFinder w;
    w.show();
    return a.exec();
}
