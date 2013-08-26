#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QPainter>
#include <QScrollBar>
#include <QTextLayout>

Editor::Editor(QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(0), mWrap(false)
{}

Editor::~Editor() {}

void Editor::paintEvent(QPaintEvent *event)
{
  QPainter painter(viewport());

  int vscroll = verticalScrollBar()->value();

  int lines = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int i = 0; i < docLines; ++i) {
    int layoutLines = layoutLine(i, layout);
    if (lines + layoutLines > vscroll) {
      qreal height = (lines - vscroll) * lineHeight();
      layout.draw(&painter, QPointF(0, height));
      if (height > viewport()->height())
        break;
    }

    lines += layoutLines;
  }

  QAbstractScrollArea::paintEvent(event);
}

void Editor::resizeEvent(QResizeEvent *event)
{
  int lines = 0;
  QTextLayout layout;
  layout.setCacheEnabled(true);
  int docLines = mDoc->lineCount();
  for (int i = 0; i < docLines; ++i)
    lines += layoutLine(i, layout);

  int visibleLines = viewport()->height() / lineHeight();
  verticalScrollBar()->setPageStep(visibleLines);
  verticalScrollBar()->setRange(0, lines - visibleLines);

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

  qreal max = std::numeric_limits<qreal>::max();
  qreal width = mWrap ? viewport()->width() : max;

  int lines = 0;
  QTextLine textLine = layout.createLine();
  while (textLine.isValid()) {
    textLine.setLineWidth(width);
    textLine.setPosition(QPointF(0, lines * lineHeight()));

    ++lines;
    textLine = layout.createLine();
  }

  layout.endLayout();
  return lines;
}
