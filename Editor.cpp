#include "Editor.h"
#include "Document.h"
#include <limits>
#include <QPainter>
#include <QTextLayout>

Editor::Editor(QWidget *parent)
  : QAbstractScrollArea(parent), mDoc(0), mWrap(false)
{}

Editor::~Editor() {}

void Editor::paintEvent(QPaintEvent *event)
{
  if (!mDoc)
    return;

  QPainter painter(viewport());

  qreal height = 0;
  qreal max = std::numeric_limits<qreal>::max();
  qreal width = mWrap ? viewport()->width() : max;

  QFont font("Consolas");
  int lines = mDoc->lineCount();
  for (int i = 0; i < lines; ++i) {
    QTextLayout layout(mDoc->textAt(i), font, this);
    layout.setCacheEnabled(true);

    qreal lineHeight = 0;
    layout.beginLayout();
    forever {
      QTextLine line = layout.createLine();
      if (!line.isValid())
        break;

      line.setLineWidth(width);

      lineHeight += line.leading();
      line.setPosition(QPointF(0, lineHeight));
      lineHeight += line.height();
    }
    layout.endLayout();

    layout.draw(&painter, QPointF(0, height));
    height += lineHeight;
  }
}
