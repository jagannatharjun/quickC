#ifndef EDITPROCESS_H
#define EDITPROCESS_H
#include <QPointer>
#include <QProcess>
#include <memory>

class QPlainTextEdit;

// a Process with plainTextEdit as output and input window
class EditProcess : public QProcess {
  Q_OBJECT
public:
  explicit EditProcess(QWidget *parent = nullptr);
  QPointer<QPlainTextEdit> edit() { return m_textEdit; }

signals:

public slots:

private:
  // std::shared_ptr<QPlainTextEdit> m_textEdit;
  QPointer<QPlainTextEdit> m_textEdit;

protected:
  bool eventFilter(QObject *, QEvent *event) override;
};

#endif // EDITPROCESS_H
