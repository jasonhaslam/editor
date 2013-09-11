#ifndef EDITOR_H
#define EDITOR_H

#include <QAbstractScrollArea>
#include <QTime>

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

struct Selection
{
  int anchor;
  int position;
};

class Editor : public QAbstractScrollArea
{
public:
  Editor(Document *doc, QWidget *parent = 0);
  virtual ~Editor();

  int length() const;
  int position() const;

  void setWrap(WrapOptions::WrapMode mode, int width = -1);

  void setPosition(int position);
  void setSelection(int anchor, int position);

  virtual void keyPressEvent(QKeyEvent *event);
  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void timerEvent(QTimerEvent *event);

private:
  int layoutLine(int line, QTextLayout &layout) const;

  Document *mDoc;
  WrapOptions mWrap;
  Selection mSelection;

  bool mCaretVisible;
  qreal mLineHeight;
};

#endif
