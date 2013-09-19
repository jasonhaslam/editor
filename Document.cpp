#include "Document.h"
#include <cassert>
#include <QStringBuilder>

namespace {
bool isUtf8ContinuationByte(unsigned char ch)
{
  return ((ch & 0xC0) == 0x80);
}
} // anon. namespace

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

int Document::columnAt(int pos) const
{
  // FIXME: Handle tab expansion.

  int column = 0;
  for (int i = lineStartPosition(lineAt(pos)); i < pos; ++i) {
    if (!isUtf8ContinuationByte(mText.at(i)))
      ++column;
  }

  return column;
}

int Document::nextColumnPosition(int pos) const
{
  int i = pos + 1;
  int len = length();
  while (i < len && isUtf8ContinuationByte(mText.at(i)))
    ++i;

  return i;
}

int Document::previousColumnPosition(int pos) const
{
  int i = pos - 1;
  while (i > 0 && isUtf8ContinuationByte(mText.at(i)))
    --i;

  return i;
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
