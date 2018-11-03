#ifndef SOURCECODEEDITOR_H
#define SOURCECODEEDITOR_H

#include <QCompleter>
#include <QDebug>
#include <QPlainTextEdit>
#include <vector>

struct CompilerMsgs {
  long int lineNo, columnNo;
  QString message;
  enum Severity { Unknown, Warning, Error } msgSeverity;
};

class SourceCodeEditor : public QPlainTextEdit {
  Q_OBJECT
public:
  SourceCodeEditor(QWidget *paren = nullptr);

  // 0: means successfull else signifies error
  int loadFile(const QString &fileName);
  int saveFile(const QString &fileName);

  void setTabSize(const int tabStop);
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  template <typename Parser> void setCompilerMsgs(Parser parser) {
    parser(m_compilerMsgs);
    qDebug() << QString("Parsing Completed got %1 results")
                    .arg(m_compilerMsgs.size());
  }

  std::vector<std::pair<QChar, QChar>> CharsToComplete() const;
  void setCharsToComplete(
      const std::vector<std::pair<QChar, QChar>> &CharsToComplete);
  void setCompleter(QCompleter *c);
  QCompleter *completer() const;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &, int);
  void insertCompletion(const QString &completion);

protected:
  void keyPressEvent(QKeyEvent *e) override;
  bool event(QEvent *) override;

  void focusInEvent(QFocusEvent *e) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  QWidget *lineNumberArea;
  std::vector<CompilerMsgs> m_compilerMsgs;
  std::vector<std::pair<QChar, QChar>> m_CharsToComplete = {
      {'{', '}'}, {'(', ')'}, {'"', '"'}};
  void moveTextCursor(QTextCursor::MoveOperation operation,
                      QTextCursor::MoveMode mode = QTextCursor::MoveAnchor,
                      int n = 1);
  QString textUnderCursor() const;
  QCompleter *c;
};

#endif // SOURCECODEEDITOR_H
