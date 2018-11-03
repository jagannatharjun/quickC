#ifndef CPPSYNTAXHIGHTLIGHTER_H
#define CPPSYNTAXHIGHTLIGHTER_H

#include <QRegularExpression>
#include <QStringList>
#include <QStringListModel>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QVector>

class CppSyntaxHightlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit CppSyntaxHightlighter(QTextDocument *parent);

  QStringListModel *wordsListModel() { return &m_wordsListModel; }

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QRegularExpression commentStartExpression;
  QRegularExpression commentEndExpression;

  QTextCharFormat keywordFormat;
  QTextCharFormat classFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat functionFormat;
  QTextCharFormat directiveFormat;

  QStringList m_words;
  QStringListModel m_wordsListModel;

  void updateWordListModel(const QString &text);

signals:

public slots:
};

#endif // CPPSYNTAXHIGHTLIGHTER_H
