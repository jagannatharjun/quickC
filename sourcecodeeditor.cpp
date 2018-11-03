#include "sourcecodeeditor.h"
#include "linenumber.h"
#include <QAbstractItemView>
#include <QDebug>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QKeyEvent>
#include <QPainter>
#include <QScrollBar>
#include <QTextBlock>
#include <QToolTip>
#include <cmath>
#include <stack>

long calculateTabLen(const QString &str) {
  long len = 0;
  for (auto c : str) {
    if (c == '{')
      len++;
    else if (c == '}')
      len--;
  }
  return std::max(len, 0l);
}

void SourceCodeEditor::setTabSize(const int tabStop) {

  QFontMetricsF fm(font());
  auto stopWidth = tabStop * fm.width(' ');
  auto letterSpacing = (ceil(stopWidth) - stopWidth) / tabStop;

  auto f = font();
  f.setLetterSpacing(QFont::AbsoluteSpacing, letterSpacing);
  setFont(f);

  setTabStopDistance(ceil(stopWidth));
}

void SourceCodeEditor::setCompleter(QCompleter *completer) {
  if (c)
    QObject::disconnect(c, 0, this, 0);

  c = completer;

  if (!c)
    return;

  c->setWidget(this);
  c->setCompletionMode(QCompleter::PopupCompletion);
  c->setCaseSensitivity(Qt::CaseInsensitive);
  QObject::connect(c, SIGNAL(activated(QString)), this,
                   SLOT(insertCompletion(QString)));
}

QCompleter *SourceCodeEditor::completer() const { return c; }

void SourceCodeEditor::insertCompletion(const QString &completion) {
  if (c->widget() != this)
    return;
  QTextCursor tc = textCursor();
  int extra = completion.length() - c->completionPrefix().length();
  tc.movePosition(QTextCursor::Left);
  tc.movePosition(QTextCursor::EndOfWord);
  tc.insertText(completion.right(extra));
  setTextCursor(tc);
}

QString SourceCodeEditor::textUnderCursor() const {
  QTextCursor tc = textCursor();
  tc.select(QTextCursor::WordUnderCursor);
  return tc.selectedText();
}

void SourceCodeEditor::focusInEvent(QFocusEvent *e) {
  if (c)
    c->setWidget(this);
  QPlainTextEdit::focusInEvent(e);
}

SourceCodeEditor::SourceCodeEditor(QWidget *paren) : QPlainTextEdit{paren} {
  lineNumberArea = new LineNumberArea(this);
  connect(this, &SourceCodeEditor::blockCountChanged, this,
          &SourceCodeEditor::updateLineNumberAreaWidth);
  connect(this, &SourceCodeEditor::updateRequest, this,
          &SourceCodeEditor::updateLineNumberArea);
  connect(this, &SourceCodeEditor::cursorPositionChanged, this,
          &SourceCodeEditor::highlightCurrentLine);
  updateLineNumberAreaWidth(0);
  highlightCurrentLine();

  const int tabStop = 4; // 4 characters
  setTabSize(tabStop);

  setCursorWidth(10);
}

int SourceCodeEditor::lineNumberAreaWidth() {
  /*
   * calculates the lineNumberArea width by counting the digits of number of
   * lines and then multiplying with size required by one digits
   */
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int charSpace = fontMetrics().horizontalAdvance(QLatin1Char('9'));
  int space = charSpace * (1 + digits);
  QPalette pl = palette();
  pl.setColor(QPalette::Text, Qt::white);
  setPalette(pl);

  return 3 + space;
}

// When we update the width of the line number area, we simply call
// QAbstractScrollArea::setViewportMargins().
void SourceCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}
/*
 * invoked when editor is scrolled
 * Rect: part of editing area to be repainted
 * dy: no of pixels the scrolled vertically
 */
void SourceCodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

// editor is resized, so resize the lineNumberAread(viewPort)
void SourceCodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

// heighlight the current aline
void SourceCodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;

    auto lineColor = QColor("#E3F2FD").darker();

    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  setExtraSelections(extraSelections);
}

void SourceCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), Qt::lightGray);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setRenderHint(QPainter::HighQualityAntialiasing);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
  int bottom = top + (int)blockBoundingRect(block).height();
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(QColor("violet").darker());
      painter.setFont(font());
      painter.drawText(0, top, lineNumberArea->width() - 3,
                       fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + (int)blockBoundingRect(block).height();
    ++blockNumber;
  }
}

int SourceCodeEditor::loadFile(const QString &fileName) {
  QFile file(fileName);
  if (!file.open(QFile::ReadOnly))
    return 1;

  document()->setPlainText(file.readAll());
  return 0;
}

int SourceCodeEditor::saveFile(const QString &fileName) {
  QFile file(fileName);
  if (!file.open(QFile::WriteOnly))
    return 1;
  file.write(document()->toPlainText().toUtf8());
  return 0;
}

void SourceCodeEditor::moveTextCursor(QTextCursor::MoveOperation operation,
                                      QTextCursor::MoveMode mode, int n) {
  auto tCursor = textCursor();
  tCursor.movePosition(operation, mode, n);
  setTextCursor(tCursor);
}

void SourceCodeEditor::keyPressEvent(QKeyEvent *e) {
  const auto keyTxt = e->text();
  if (c && c->popup()->isVisible()) {
    // The following keys are forwarded by the completer to the widget
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
    case Qt::Key_Escape:
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
      e->ignore();
      return; // let the completer do default behavior
    default:
      break;
    }
  }

  bool isShortcut = ((e->modifiers() & Qt::ControlModifier) &&
                     e->key() == Qt::Key_E); // CTRL+E
  if ((!c || !isShortcut) &&
      (c && !c->popup()->isVisible())) { // do not process the shortcut
                                         // when we have a completer
    static std::stack<QChar> keysToEat;
    if (keyTxt == "\n" || keyTxt == "\r") {
      insertPlainText(e->text() + QString(calculateTabLen(toPlainText().left(
                                              textCursor().position())),
                                          '\t'));
      return;
    }
    for (const auto &c : m_CharsToComplete) {
      if (c.first == keyTxt) {
        insertPlainText(e->text() + c.second);
        moveTextCursor(QTextCursor::PreviousCharacter);
        keysToEat.push(c.second);
        return;
      }
    }
    if (!keysToEat.empty() && keysToEat.top() == keyTxt) {
      moveTextCursor(QTextCursor::NextCharacter);
      keysToEat.pop();
      return;
    }

    QPlainTextEdit::keyPressEvent(e);
  }

  const bool ctrlOrShift =
      e->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
  if (!c || (ctrlOrShift && e->text().isEmpty()))
    return;

  static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
  bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
  QString completionPrefix = textUnderCursor();

  if (!isShortcut &&
      (hasModifier || e->text().isEmpty() || completionPrefix.length() < 3 ||
       eow.contains(e->text().right(1)))) {
    c->popup()->hide();
    return;
  }

  if (completionPrefix != c->completionPrefix()) {
    c->setCompletionPrefix(completionPrefix);
    c->popup()->setCurrentIndex(c->completionModel()->index(0, 0));
  }
  QRect cr = cursorRect();
  cr.setWidth(c->popup()->sizeHintForColumn(0) +
              c->popup()->verticalScrollBar()->sizeHint().width());
  c->complete(cr); // popup it up!
}

bool SourceCodeEditor::event(QEvent *event) {
  if (event->type() == QEvent::ToolTip) {
    qDebug() << "Tool tip Event";
    auto *helpEvent = dynamic_cast<QHelpEvent *>(event);
    QTextCursor cursor = cursorForPosition(helpEvent->pos());
    auto cursorLineNo = cursor.blockNumber() + 1;
    for (const auto &msg : m_compilerMsgs) {
      if (msg.lineNo == cursorLineNo) {
        auto previousStyleSheet = this->styleSheet();
        if (msg.msgSeverity == CompilerMsgs::Severity::Error) {
          setStyleSheet("QToolTip { color: #B00020; }");
        } else
          setStyleSheet("QToolTip { color: yellow; }");
        QToolTip::showText(helpEvent->globalPos(), msg.message, this);
        setStyleSheet(previousStyleSheet);
        return true;
      }
    }
    QToolTip::hideText();
    event->setAccepted(true);
    return true;
  }
  return QPlainTextEdit::event(event);
}

std::vector<std::pair<QChar, QChar>> SourceCodeEditor::CharsToComplete() const {
  return m_CharsToComplete;
}

void SourceCodeEditor::setCharsToComplete(
    const std::vector<std::pair<QChar, QChar>> &CharsToComplete) {
  m_CharsToComplete = CharsToComplete;
}
