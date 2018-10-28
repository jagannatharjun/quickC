#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  void setShortCuts();

  void setMenuEdit();

private:
  Ui::MainWindow *ui;

  struct _Detail;
  std::unique_ptr<_Detail> details;

  void menuFileTriggered(QAction *);
  void arrangeCentralWidgetElements();
  void setMenuCompile();
};

#endif // MAINWINDOW_H
