#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

Editor::Editor(Document *doc, QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(doc),
    mCaretTimerId(0), mCaretVisible(false),
    mLineHeight(QFontMetricsF(font()).lineSpacing())
{
  startCaretTimer();
  setSelection(0, 0);
  setWrap(WrapOptions::WrapNone);
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

void Editor::setPosition(int position)
{
  setSelection(position, position);
}

void Editor::setSelection(int anchor, int position)
{
  mSelection.anchor = qBound(0, anchor, length());
  mSelection.position = qBound(0, position, length());
}

void Editor::keyPressEvent(QKeyEvent *event)
{
  switch (event->key()) {
    case Qt::Key_Left: {
      setPosition(mDoc->previousColumnPosition(position()));
      break;
    }

    case Qt::Key_Right: {
      setPosition(mDoc->nextColumnPosition(position()));
      break;
    }

    case Qt::Key_Up: {
      int line = mDoc->lineAt(position());
      int column = mDoc->columnAt(position());

      // Lay out the current line.
      QTextLayout layout;
      layoutLine(line, layout);
      QTextLine textLine = layout.lineForTextPosition(column);
      qreal x = textLine.cursorToX(column);

      if (textLine.lineNumber() > 0) {
        // This is a wrapped line.
        QTextLine prevLine = layout.lineAt(textLine.lineNumber() - 1);
        setPosition(mDoc->columnPosition(line, prevLine.xToCursor(x)));
      } else if (line > 0) {
        // Lay out the previous line.
        int layoutLines = layoutLine(line - 1, layout);
        QTextLine prevLine = layout.lineAt(layoutLines - 1);
        setPosition(mDoc->columnPosition(line - 1, prevLine.xToCursor(x)));
      }

      break;
    }

    case Qt::Key_Down: {
      int line = mDoc->lineAt(position());
      int column = mDoc->columnAt(position());

      // Lay out the current line.
      QTextLayout layout;
      int layoutLines = layoutLine(line, layout);
      QTextLine textLine = layout.lineForTextPosition(column);
      qreal x = textLine.cursorToX(column);

      if (textLine.lineNumber() < layoutLines - 1) {
        // There's a wrapped line below this one.
        QTextLine nextLine = layout.lineAt(textLine.lineNumber() + 1);
        setPosition(mDoc->columnPosition(line, nextLine.xToCursor(x)));
      } else if (line < lineCount() - 1) {
        // Lay out the next line.
        layoutLine(line + 1, layout);
        QTextLine nextLine = layout.lineAt(0);
        setPosition(mDoc->columnPosition(line + 1, nextLine.xToCursor(x)));
      }

      break;
    }
  }

  // Ensure that caret is visible.
  startCaretTimer();

  // FIXME: Constrain update?
  viewport()->update();
}

void Editor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int minY = event->rect().y();
  int maxY = minY + event->rect().height();
  int vscroll = verticalScrollBar()->value();
  int hscroll = horizontalScrollBar()->value();

  int caretPos = mSelection.position;
  int caretLine = mDoc->lineAt(caretPos);

  int minPos = qMin(mSelection.anchor, mSelection.position);
  int maxPos = qMax(mSelection.anchor, mSelection.position);
  int minLine = mDoc->lineAt(minPos);
  int maxLine = mDoc->lineAt(maxPos);

  // Layout through the end of the view.
  int lines = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int line = 0; line < docLines; ++line) {
    qreal y = (lines - vscroll) * mLineHeight;
    if (y > maxY) // Nothing more to draw.
      break;

    int layoutLines = layoutLine(line, layout);
    if (y + (layoutLines * mLineHeight) > minY) {
      QVector<QTextLayout::FormatRange> selections;
      if (minPos != maxPos && line >= minLine && line <= maxLine) {
        int len = layout.text().length();
        int start = (line == minLine) ? mDoc->columnAt(minPos) : 0;
        int end = (line == maxLine) ? mDoc->columnAt(maxPos) : len;

        QTextLayout::FormatRange range;
        range.start = start;
        range.length = end - start;
        range.format.setBackground(palette().highlight());
        range.format.setForeground(palette().highlightedText());
        selections.append(range);
      }

      QPointF point(-hscroll, y);
      layout.draw(&painter, point, selections);

      if (mCaretVisible && line == caretLine)
        layout.drawCursor(&painter, point, mDoc->columnAt(caretPos));
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

  int visibleLines = viewport()->height() / mLineHeight;
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
    qreal y = (lines - vscroll) * mLineHeight;
    if (y > maxY) // The caret is beyond the view.
      return;

    lines += layoutLine(line, layout);
  }

  // Layout the caret line.
  qreal y = (lines - vscroll) * mLineHeight;
  int height = layoutLine(caretLine, layout) * mLineHeight;
  if (y + height > 0)
    viewport()->update(QRect(0, y, viewport()->width(), height));
}

void Editor::startCaretTimer()
{
  mCaretVisible = true;
  killTimer(mCaretTimerId);
  mCaretTimerId = startTimer(QApplication::cursorFlashTime() / 2);
}

int Editor::layoutLine(int line, QTextLayout &layout) const
{
  layout.setText(mDoc->lineText(line));
  layout.beginLayout();

  int lines = 0;
  QTextLine textLine = layout.createLine();
  while (textLine.isValid()) {
    textLine.setLineWidth(mWrap.width);
    textLine.setPosition(QPointF(0, lines * mLineHeight));

    ++lines;
    textLine = layout.createLine();
  }

  layout.endLayout();
  return lines;
}
