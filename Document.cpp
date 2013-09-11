#include "Document.h"
#include <cassert>
#include <QStringBuilder>

Document::Document()
{
  mLines.insert(0, 0);
}

Document::~Document() {}

int Document::lineAt(int pos) const
{
  int min = 0;
  int max = lineCount() - 1;
  while (min < max) {
    int mid = (min + max) / 2;
    if (mLines.at(mid) < pos) {
      min = mid + 1;
    } else {
      max = mid;
    }
  }

  return (mLines.at(min) > pos) ? min - 1 : min;
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
  assert(line >= 0 && line < lineCount());
  int pos = lineStartPosition(line);
  return text(pos, lineEndPosition(line) - pos + 1);
}

QString Document::text(int pos, int len) const
{
  int gap = mText.gapPosition();
  if (pos >= gap || pos + len < gap)
    return QString::fromUtf8(mText.constData(pos), len);

  int sublen = gap - pos;
  return QString::fromUtf8(mText.constData(pos), sublen) %
         QString::fromUtf8(mText.constData(gap), len - sublen);
}

QByteArray Document::lineCaretPositions(int line) const
{
  assert(line >= 0 && line < lineCount());
  int pos = lineStartPosition(line);
  return caretPositions(pos, lineEndPosition(line) - pos + 1);
}

QByteArray Document::caretPositions(int pos, int len) const
{
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
