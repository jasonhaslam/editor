#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

Editor::Editor(QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(0), mCaretVisible(false)
{
  setWrap(WrapOptions::WrapNone);
  setSelection(0, 0);

  startTimer(QApplication::cursorFlashTime() / 2);
}

Editor::~Editor() {}

void Editor::setWrap(WrapOptions::WrapMode mode, int width)
{
  mWrap.mode = mode;
  switch (mode) {
    case WrapOptions::WrapNone:
      mWrap.width = std::numeric_limits<int>::max();
      break;

    case WrapOptions::WrapWidget:
      mWrap.width = viewport()->width();
      break;

    case WrapOptions::WrapFixed:
      mWrap.width = width;
      break;
  }
}

void Editor::setSelection(int anchor, int position)
{
  mSelection.anchor = anchor;
  mSelection.position = position;
}

void Editor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int minY = event->rect().y();
  int maxY = minY + event->rect().height();
  int vscroll = verticalScrollBar()->value();
  int hscroll = horizontalScrollBar()->value();

  int caretLine = mDoc->lineAt(mSelection.position);
  int caretOffset = mSelection.position - mDoc->lineStartPosition(caretLine);

  int minPos = qMin(mSelection.anchor, mSelection.position);
  int maxPos = qMax(mSelection.anchor, mSelection.position);
  int minLine = mDoc->lineAt(minPos);
  int maxLine = mDoc->lineAt(maxPos);
  int minOffset = minPos - mDoc->lineStartPosition(minLine);
  int maxOffset = maxPos - mDoc->lineStartPosition(maxLine);

  // Layout through the end of the view.
  int lines = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int line = 0; line < docLines; ++line) {
    qreal y = (lines - vscroll) * lineHeight();
    if (y > maxY) // Nothing more to draw.
      break;

    int layoutLines = layoutLine(line, layout);
    if (y + (layoutLines * lineHeight()) > minY) {
      QVector<QTextLayout::FormatRange> selections;
      if (minPos != maxPos && line >= minLine && line <= maxLine) {
        int len = layout.text().length();
        QByteArray offsets = mDoc->lineOffsets(line);
        int start = (line == minLine) ? offsets.at(minOffset) : 0;
        int end = (line == maxLine) ? offsets.at(maxOffset) : len;

        QTextLayout::FormatRange range;
        range.start = start;
        range.length = end - start;
        range.format.setBackground(palette().highlight());
        range.format.setForeground(palette().highlightedText());
        selections.append(range);
      }

      QPointF point(-hscroll, y);
      layout.draw(&painter, point, selections);

      if (mCaretVisible && line == caretLine) {
        QByteArray offsets = mDoc->lineOffsets(line);
        layout.drawCursor(&painter, point, offsets.at(caretOffset));
      }
    }

    lines += layoutLines;
  }
}

void Editor::resizeEvent(QResizeEvent *event)
{
  if (mWrap.mode == WrapOptions::WrapWidget)
    mWrap.width = viewport()->width();

  // Layout all lines.
  int lines = 0;
  int width = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int line = 0; line < docLines; ++line) {
    lines += layoutLine(line, layout);
    width = qMax<int>(width, layout.boundingRect().width());
  }

  int visibleLines = viewport()->height() / lineHeight();
  verticalScrollBar()->setPageStep(visibleLines);
  verticalScrollBar()->setRange(0, lines - visibleLines);

  int visibleWidth = viewport()->width();
  horizontalScrollBar()->setPageStep(visibleWidth);
  horizontalScrollBar()->setRange(0, width - visibleWidth);
}

void Editor::timerEvent(QTimerEvent *event)
{
  mCaretVisible = !mCaretVisible;

  // Find the caret line in the view.
  int maxY = viewport()->height();
  int vscroll = verticalScrollBar()->value();

  // Layout up to the caret line.
  int lines = 0;
  QTextLayout layout;
  int caretLine = mDoc->lineAt(mSelection.position);
  for (int line = 0; line < caretLine; ++line) {
    qreal y = (lines - vscroll) * lineHeight();
    if (y > maxY) // The caret is beyond the view.
      return;

    lines += layoutLine(line, layout);
  }

  // Layout the caret line.
  qreal y = (lines - vscroll) * lineHeight();
  int height = layoutLine(caretLine, layout) * lineHeight();
  if (y + height > 0)
    viewport()->update(QRect(0, y, viewport()->width(), height));
}

qreal Editor::lineHeight() const
{
  return QFontMetricsF(font()).lineSpacing();
}

int Editor::layoutLine(int line, QTextLayout &layout) const
{
  layout.setText(mDoc->lineText(line));
  layout.beginLayout();

  int lines = 0;
  QTextLine textLine = layout.createLine();
  while (textLine.isValid()) {
    textLine.setLineWidth(mWrap.width);
    textLine.setPosition(QPointF(0, lines * lineHeight()));

    ++lines;
    textLine = layout.createLine();
  }

  layout.endLayout();
  return lines;
}
