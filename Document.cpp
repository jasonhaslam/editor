#include "Document.h"
#include <QStringBuilder>

Document::Document()
{
  mLines.insert(0, 0);
}

Document::~Document() {}

int Document::lineAt(int pos) const
{
  int lines = lineCount();
  int line = lines / 2;
  while (line > 0 && line < lines - 1) {
    if (pos < mLines.at(line)) {
      line /= 2;
    } else if (pos >= mLines.at(line + 1)) {
      line += (lines - line) / 2;
    } else {
      return line;
    }
  }

  return line;
}

QString Document::textAt(int line) const
{
  int lines = lineCount();
  if (line < 0 || line >= lines)
    return QString();

  int pos = mLines.at(line);
  return text(pos, (line < lines - 1) ? (mLines.at(line + 1) - pos) : -1);
}

QString Document::text(int pos, int len) const
{
  if (len < 0)
    len = length() - pos;

  int gap = mText.gapPosition();
  if (pos >= gap || pos + len < gap)
    return QString::fromUtf8(mText.constData(pos), len);

  int sublen = gap - pos;
  return QString::fromUtf8(mText.constData(pos), sublen) %
         QString::fromUtf8(mText.constData(gap), len - sublen);
}

void Document::insert(int pos, const QString &text)
{
  QByteArray utf8 = text.toUtf8();
  mText.insert(pos, utf8.constData(), utf8.length());

  int len = utf8.length();
  int lines = lineCount();
  int line = lineAt(pos) + 1;
  for (int i = line; i < lines; ++i)
    mLines[line] += len;

  for (int i = len - 1; i >= 0; --i) {
    if (utf8.at(i) == '\n')
      mLines.insert(line, pos + i + 1);
  }
}

void Document::remove(int pos, int len)
{
  if (len < 0)
    len = length();

  int lines = lineCount();
  int line = lineAt(pos) + 1;
  for (int i = line; i < lines; ++i) {
    int &linePos = mLines[i];
    if (linePos < pos + len) {
      mLines.remove(i);
    } else {
      linePos -= len;
    }
  }

  mText.remove(pos, len);
}
