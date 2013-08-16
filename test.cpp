#include "Document.h"
#include "Editor.h"
#include "GapBuffer.h"
#include <QApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QTextStream out(stdout);

  GapBuffer<char> buf;
  const char *str1 = "Hello world!";
  int len1 = strlen(str1);
  buf.insert(0, str1, len1);
  out << buf << endl;

  const char *str2 = "brave new";
  int len2 = strlen(str2);
  buf.insert(6, str2, len2);
  buf.insert(6 + len2, ' ');
  out << buf << endl;

  buf.remove(0, 6);
  buf.insert(0, ' ');
  buf.insert(0, 'O');
  out << buf << endl;

  const char *str3 = "that has such people in't.";
  int len3 = strlen(str3);
  buf.remove(buf.length() - 1, 1);
  buf.insert(buf.length(), ',');
  buf.insert(buf.length(), ' ');
  buf.insert(buf.length(), str3, len3);
  out << buf << endl;

  buf[0] = '*';
  out << buf << endl;
  out << "buf[0]: " << buf.at(0) << endl;
  out << QString::fromUtf8(buf.data(2, 15)) << endl;

  QString str4 =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco\n"
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n"
    "dolor in reprehenderit in voluptate velit esse cillum dolore\n"
    "eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non\n"
    "proident, sunt in culpa qui officia deserunt mollit anim id est\n"
    "laborum.";
  Document doc;
  doc.insert(0, str4);
  out << endl << doc << endl;

  Editor editor;
  editor.setWrap(true);
  editor.setDocument(&doc);
  editor.resize(640, 480);
  editor.show();

  return app.exec();
}
