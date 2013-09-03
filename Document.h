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

  QString lineText(int line) const;
  QString text(int pos = 0, int len = -1) const;

  QByteArray lineOffsets(int line) const;
  QByteArray offsets(int pos = 0, int len = -1) const;

  void insert(int pos, const QString &text);
  void remove(int pos, int len = -1);

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
