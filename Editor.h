#ifndef EDITOR_H
#define EDITOR_H

#include <QAbstractScrollArea>

class Document;
class QTextLayout;

class Editor : public QAbstractScrollArea
{
public:
  Editor(QWidget *parent = 0);
  virtual ~Editor();

  void setDocument(Document *doc) { mDoc = doc; }
  void setWrap(bool enabled) { mWrap = enabled; }

  virtual void paintEvent(QPaintEvent *event);
  virtual void resizeEvent(QResizeEvent *event);

private:
  qreal lineHeight() const;
  int layoutLine(int line, QTextLayout &layout) const;

  Document *mDoc;
  bool mWrap;
};

#endif
