#include "main_text_finder_window.h"
#include "./ui_main_text_finder_window.h"
#include <QFile>
#include <QTextStream>
#include <LoggerCpp/LoggerCpp.h>

TextFinder::TextFinder(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::TextFinder)
{

    Log::Logger logger("text_finder");
    logger.notice() << "Before setupUi";
    ui->setupUi(this);
    logger.notice() << "Before loadTextFile";
    loadTextFile();
}

TextFinder::~TextFinder()
{
    delete ui;
}

void TextFinder::loadTextFile()
{
    QFile inputFile("input.txt");
    inputFile.open(QIODevice::ReadOnly);

    QTextStream in(&inputFile);
    QString line = in.readAll();
    inputFile.close();

    ui->textEdit->setPlainText(line);
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor, 1);
}

void TextFinder::on_findButton_clicked()
{
    QString searchString = ui->lineEdit->text();
    ui->textEdit->find(searchString, QTextDocument::FindWholeWords);
}
