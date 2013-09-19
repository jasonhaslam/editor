#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "GapBuffer.h"
#include <QString>
#include <QTextStream>

class Document
{
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

  QString lineText(int line) const;
  QString text(int pos, int len) const;

  void insert(int pos, const QString &text);
  void remove(int pos, int len);

private:
  GapBuffer<char> mText;
  GapBuffer<int> mLines;

  friend QTextStream &operator<<(QTextStream &lhs, Document &rhs);
};

inline QTextStream &operator<<(QTextStream &lhs, Document &rhs)
{
  int lines = rhs.lineCount();
  for (int line = 0; line < lines; ++line)
    lhs << rhs.lineText(line);
  return lhs;
}

#endif
