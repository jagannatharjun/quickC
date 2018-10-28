#include "editprocess.h"
#include <QDebug>
#include <QKeyEvent>
#include <QPlainTextEdit>

EditProcess::EditProcess(QWidget *parent)
    : QProcess(parent), m_textEdit{new QPlainTextEdit}
// zm_textEdit(std::make_shared<QPlainTextEdit>(parent))
{
  QObject::connect(this, &EditProcess::readyReadStandardOutput, [this]() {
    m_textEdit->insertPlainText(readAllStandardOutput());
  });

  QObject::connect(this, &EditProcess::readyReadStandardError, [this]() {
    m_textEdit->insertPlainText(readAllStandardError());
  });

  QObject::connect(
      this,
      static_cast<void (EditProcess::*)(int, QProcess::ExitStatus)>(
          &EditProcess::finished),
      [this](int exitCode, QProcess::ExitStatus) -> void {
        m_textEdit->insertPlainText(
            QString("\nProgram Finished with exit code: %1").arg(exitCode));
      });

  m_textEdit->installEventFilter(this);
}

bool EditProcess::eventFilter(QObject *, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    write(dynamic_cast<QKeyEvent *>(event)->text().toUtf8());
  }
  return false;
}
