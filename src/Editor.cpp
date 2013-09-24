#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QApplication>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

Editor::Editor(Document *doc, QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(doc), mLastX(0),
    mCaretPeriod(QApplication::cursorFlashTime() / 2),
    mCaretTimerId(0), mCaretVisible(false),
    mLineHeight(QFontMetricsF(font()).lineSpacing())
{
  // Set widget defaults.
  viewport()->setCursor(Qt::IBeamCursor);

  // Set editor defaults.
  setWrap(WrapNone);
  updateCaret();
}

Editor::~Editor() {}

int Editor::positionAt(const QPoint &point) const
{
  int minY = qMax(0, point.y());
  int vscroll = verticalScrollBar()->value();
  int hscroll = horizontalScrollBar()->value();

  // Layout to point.
  int lines = 0;
  QTextLayout layout;
  int docLines = mDoc->lineCount();
  for (int line = 0; line < docLines; ++line) {
    qreal y = (lines - vscroll) * mLineHeight;
    int layoutLines = layoutLine(line, layout);
    if (y + (layoutLines * mLineHeight) > minY) {
      QTextLine textLine = layout.lineAt((minY - y) / mLineHeight);
      return mDoc->positionAt(line, textLine.xToCursor(hscroll + point.x()));
    }

    lines += layoutLines;
  }

  // FIXME: Return the correct x position in that last visible line.
  return length();
}

void Editor::setWrap(WrapMode mode, int width)
{
  mWrap.mode = mode;
  switch (mode) {
    case WrapNone:
      mWrap.width = std::numeric_limits<int>::max();
      break;

    case WrapWidget:
      mWrap.width = viewport()->width();
      break;

    case WrapFixed:
      mWrap.width = width;
      break;
  }
}

void Editor::setPosition(int pos, MoveMode mode)
{
  setSelection((mode == MoveAnchor) ? pos : anchor(), pos);
}

void Editor::setSelection(int anchor, int position)
{
  mSelection.anchor = qBound(0, anchor, length());
  mSelection.position = qBound(0, position, length());
}

void Editor::keyPressEvent(QKeyEvent *event)
{
  bool shift = (event->modifiers() & Qt::ShiftModifier);
  MoveMode mode = shift ? KeepAnchor : MoveAnchor;

  bool deselect = (anchor() != position() && !shift);
  int min = qMin(anchor(), position());
  int max = qMax(anchor(), position());

  switch (event->key()) {
    case Qt::Key_Left: {
      int pos = deselect ? min : mDoc->previousColumnPosition(position());
      setPosition(pos, mode);
      rememberLastX(pos);
      break;
    }

    case Qt::Key_Right: {
      int pos = deselect ? max : mDoc->nextColumnPosition(position());
      setPosition(pos, mode);
      rememberLastX(pos);
      break;
    }

    case Qt::Key_Up: {
      int pos = deselect ? min : position();
      int line = mDoc->lineAt(pos);
      int column = mDoc->columnAt(pos);

      // Layout the current line.
      QTextLayout layout;
      layoutLine(line, layout);
      QTextLine textLine = layout.lineForTextPosition(column);
      qreal x = qMax(textLine.cursorToX(column), mLastX);

      if (textLine.lineNumber() > 0) {
        // This is a wrapped line.
        QTextLine prev = layout.lineAt(textLine.lineNumber() - 1);
        setPosition(mDoc->positionAt(line, prev.xToCursor(x)), mode);
      } else if (line > 0) {
        // Layout the previous line.
        int layoutLines = layoutLine(line - 1, layout);
        QTextLine prev = layout.lineAt(layoutLines - 1);
        setPosition(mDoc->positionAt(line - 1, prev.xToCursor(x)), mode);
      } else {
        // This is the first line.
        setPosition(0, mode);
      }

      break;
    }

    case Qt::Key_Down: {
      int pos = deselect ? max : position();
      int line = mDoc->lineAt(pos);
      int column = mDoc->columnAt(pos);

      // Layout the current line.
      QTextLayout layout;
      int layoutLines = layoutLine(line, layout);
      QTextLine textLine = layout.lineForTextPosition(column);
      qreal x = qMax(textLine.cursorToX(column), mLastX);

      if (textLine.lineNumber() < layoutLines - 1) {
        // There's a wrapped line below this one.
        QTextLine next = layout.lineAt(textLine.lineNumber() + 1);
        setPosition(mDoc->positionAt(line, next.xToCursor(x)), mode);
      } else if (line < lineCount() - 1) {
        // Layout the next line.
        layoutLine(line + 1, layout);
        QTextLine next = layout.lineAt(0);
        setPosition(mDoc->positionAt(line + 1, next.xToCursor(x)), mode);
      } else {
        // This is the last line.
        setPosition(length(), mode);
      }

      break;
    }
  }

  // FIXME: Constrain update?
  updateCaret();
  viewport()->update();
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
  int pos = positionAt(event->pos());
  if (pos >= 0)
    setPosition(pos, KeepAnchor);

  // FIXME: Constrain update?
  updateCaret();
  viewport()->update();
}

void Editor::mousePressEvent(QMouseEvent *event)
{
  int pos = positionAt(event->pos());
  if (pos >= 0)
    setPosition(pos);

  // FIXME: Constrain update?
  updateCaret();
  viewport()->update();
}

void Editor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int minY = event->rect().y();
  int maxY = minY + event->rect().height();
  int vscroll = verticalScrollBar()->value();
  int hscroll = horizontalScrollBar()->value();

  int caretPos = position();
  int caretLine = mDoc->lineAt(caretPos);

  int minPos = qMin(anchor(), position());
  int maxPos = qMax(anchor(), position());
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
  if (mWrap.mode == WrapWidget)
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
  int caretLine = mDoc->lineAt(position());
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

void Editor::updateCaret()
{
  killTimer(mCaretTimerId);
  mCaretVisible = (anchor() == position());
  mCaretTimerId = mCaretVisible ? startTimer(mCaretPeriod) : 0;
}

void Editor::rememberLastX(int pos)
{
  int line = mDoc->lineAt(pos);
  int column = mDoc->columnAt(pos);

  QTextLayout layout;
  layoutLine(line, layout);

  QTextLine textLine = layout.lineForTextPosition(column);
  mLastX = textLine.cursorToX(column);
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
