#ifndef EDITOR_H
#define EDITOR_H

#include "Document.h"
#include <QAbstractScrollArea>
#include <QTime>

class QTextLayout;

class Editor : public QAbstractScrollArea
{
  Q_OBJECT

public:
  enum WrapMode
  {
    WrapNone,   // Don't wrap.
    WrapWidget, // Wrap at widget's edge.
    WrapFixed   // Wrap at a fixed width.
  };

  enum MoveMode
  {
    MoveAnchor,
    KeepAnchor
  };

  struct WrapOptions
  {
    WrapOptions()
      : mode(WrapNone), width(0)
    {}

    WrapMode mode;
    int width;
  };

  struct Selection
  {
    Selection()
      : anchor(0), position(0)
    {}

    int anchor;
    int position;
  };

  Editor(Document *doc, QWidget *parent = 0);
  virtual ~Editor();

  int length() const { return mDoc->length(); }
  int lineCount() const { return mDoc->lineCount(); }

  int anchor() const { return mSelection.anchor; }
  int position() const { return mSelection.position; }

  int positionAt(const QPoint &point) const;

  void setWrap(WrapMode mode, int width = -1);

  void setPosition(int position, MoveMode mode = MoveAnchor);
  void setSelection(int anchor, int position);

protected:
  virtual void keyPressEvent(QKeyEvent *event);
  virtual void mouseMoveEvent(QMouseEvent *event);
  virtual void mousePressEvent(QMouseEvent *event);
  virtual void mouseReleaseEvent(QMouseEvent *event);
  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);
  virtual void timerEvent(QTimerEvent *event);

private:
  void updateCaret();
  void rememberLastX();

  int layoutLine(int line, QTextLayout &layout) const;

private slots:
  void updateLineCache(int line, int linesAdded);

private:
  Document *mDoc;
  WrapOptions mWrap;

  qreal mLastX;
  Selection mSelection;

  int mCaretPeriod;
  int mCaretTimerId;
  bool mCaretVisible;

  qreal mLineHeight;
  GapBuffer<int> mLines;
};

#endif
