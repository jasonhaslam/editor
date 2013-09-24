#include "Document.h"
#include "Editor.h"
#include "GapBuffer.h"
#include <QApplication>
#include <QTextStream>

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);

  Document doc;

  doc.append(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco\n"
    "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n"
    "dolor in reprehenderit in voluptate velit esse cillum dolore\n"
    "eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat non\n"
    "proident, sunt in culpa qui officia deserunt mollit anim id est\n"
    "laborum.\n\n");

  doc.append(
    "int main(int argc, const char *argv[]) {\n"
    "\treturn 0;\n"
    "}\n");

  Editor editor(&doc);
  editor.resize(320, 240);
  editor.setWrap(Editor::WrapWidget);
  editor.show();

  return app.exec();
}
