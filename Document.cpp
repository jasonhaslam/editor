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

int Document::lineEndPosition(int line) const
{
  return (line < lineCount() - 1) ? mLines.at(line + 1) - 1 : length();
}

int Document::lineStartPosition(int line) const
{
  return mLines.at(line);
}

QString Document::lineText(int line) const
{
  if (line < 0 || line >= lineCount())
    return QString();

  int pos = lineStartPosition(line);
  return text(pos, lineEndPosition(line) - pos);
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

QByteArray Document::lineOffsets(int line) const
{
  if (line < 0 || line >= lineCount())
    return QByteArray();
  
  int pos = lineStartPosition(line);
  return offsets(pos, lineEndPosition(line) - pos);
}

QByteArray Document::offsets(int pos, int len) const
{
  if (len < 0)
    len = length() - pos;

  char index = 0;
  QByteArray indexes;
  indexes.reserve(len + 1);
  for (int i = 0; i < len; ++i) {
    unsigned char ch = mText.at(i);
    indexes[i] = index;
    if (ch >= 0xC0)
      indexes[i++] = index;
    if (ch >= 0xE0)
      indexes[i++] = index;
    if (ch >= 0xF0)
      indexes[i++] = index;
    ++index;
  }

  return indexes;
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
    len = length() - pos;

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
