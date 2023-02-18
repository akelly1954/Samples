#pragma once

#include <QWidget>

namespace Ui {
  class GraphicsViewWidget;
}

class GraphicsViewWidget : public QWidget
{
  Q_OBJECT

public:
  explicit GraphicsViewWidget(QWidget *parent = nullptr);
  ~GraphicsViewWidget();

private:
  Ui::GraphicsViewWidget *ui;
};

