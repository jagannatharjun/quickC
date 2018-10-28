#ifndef LINENUMBER_H
#define LINENUMBER_H

#include "sourcecodeeditor.h"

class LineNumberArea : public QWidget {
  Q_OBJECT
public:
  LineNumberArea(SourceCodeEditor *editor) : QWidget(editor) {
    codeEditor = editor;
  }

  QSize sizeHint() const override {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

private:
  SourceCodeEditor *codeEditor;
};
#endif // LINENUMBER_H
