#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

Editor::Editor(QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(0)
{
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

void Editor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int vscroll = verticalScrollBar()->value();
  int hscroll = horizontalScrollBar()->value();

  int lines = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int i = 0; i < docLines; ++i) {
    int layoutLines = layoutLine(i, layout);
    if (lines + layoutLines > vscroll) {
      qreal y = (lines - vscroll) * lineHeight();
      layout.draw(&painter, QPointF(-hscroll, y));
      if (y > viewport()->height())
        break;
    }

    lines += layoutLines;
  }

  QAbstractScrollArea::paintEvent(event);
}

void Editor::resizeEvent(QResizeEvent *event)
{
  if (mWrap.mode == WrapOptions::WrapWidget)
    mWrap.width = viewport()->width();

  int lines = 0;
  int width = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int i = 0; i < docLines; ++i) {
    lines += layoutLine(i, layout);
    width = qMax<int>(width, layout.boundingRect().width());
  }

  int visibleLines = viewport()->height() / lineHeight();
  verticalScrollBar()->setPageStep(visibleLines);
  verticalScrollBar()->setRange(0, lines - visibleLines);

  int visibleWidth = viewport()->width();
  horizontalScrollBar()->setPageStep(visibleWidth);
  horizontalScrollBar()->setRange(0, width - visibleWidth);

  QAbstractScrollArea::resizeEvent(event);
}

qreal Editor::lineHeight() const
{
  return QFontMetricsF(font()).lineSpacing();
}

int Editor::layoutLine(int line, QTextLayout &layout) const
{
  layout.setText(mDoc->textAt(line));
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
