#ifndef EDITOR_H
#define EDITOR_H

#include <QAbstractScrollArea>

class Document;
class QTextLayout;

struct WrapOptions
{
  enum WrapMode {
    WrapNone,   // Don't wrap.
    WrapWidget, // Wrap at widget's edge.
    WrapFixed   // Wrap at a fixed width.
  };

  WrapMode mode;
  int width;
};

class Editor : public QAbstractScrollArea
{
public:
  Editor(QWidget *parent = 0);
  virtual ~Editor();

  void setDocument(Document *doc) { mDoc = doc; }
  void setWrap(WrapOptions::WrapMode mode, int width = -1);

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);

private:
  qreal lineHeight() const;
  int layoutLine(int line, QTextLayout &layout) const;

  Document *mDoc;
  WrapOptions mWrap;
};

#endif
