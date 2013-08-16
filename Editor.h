#ifndef EDITOR_H
#define EDITOR_H

#include <QAbstractScrollArea>

class Document;

class Editor : public QAbstractScrollArea
{
public:
  Editor(QWidget *parent = 0);
  virtual ~Editor();

  Document *document() const { return mDoc; }
  void setDocument(Document *doc) { mDoc = doc; }

  bool wrap() const { return mWrap; }
  void setWrap(bool enabled) { mWrap = enabled; }

  virtual void paintEvent(QPaintEvent *event);

private:
  Document *mDoc;
  bool mWrap;
};

#endif
