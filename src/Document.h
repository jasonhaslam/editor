#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "GapBuffer.h"
#include <QObject>
#include <QString>
#include <QTextStream>

class Document : public QObject
{
  Q_OBJECT

public:
  Document();
  virtual ~Document();

  int length() const { return mText.length(); }
  int lineCount() const { return mLines.length(); }

  int lineAt(int pos) const;
  int lineEndPosition(int line) const;
  int lineStartPosition(int line) const;

  int columnAt(int pos) const;
  int nextColumnPosition(int pos) const;
  int previousColumnPosition(int pos) const;

  int positionAt(int line, int column) const;

  QString lineText(int line) const;
  QString text(int pos, int len) const;

  void append(const QString &text);
  void insert(int pos, const QString &text);
  void remove(int pos, int len);

signals:
  void linesChanged(int line, int linesAdded);

private:
  GapBuffer<char> mText;
  GapBuffer<int> mLines;

  friend QTextStream &operator<<(QTextStream &lhs, Document &rhs);
};

inline QTextStream &operator<<(QTextStream &lhs, Document &rhs)
{
  int lines = rhs.lineCount();
  for (int line = 0; line < lines; ++line)
    lhs << rhs.lineText(line) << endl;
  return lhs;
}

#endif
