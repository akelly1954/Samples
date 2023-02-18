#include "graphicsviewwidget.h"
#include "ui_graphicsviewwidget.h"

GraphicsViewWidget::GraphicsViewWidget(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::GraphicsViewWidget)
{
  ui->setupUi(this);
}

GraphicsViewWidget::~GraphicsViewWidget()
{
  delete ui;
}
