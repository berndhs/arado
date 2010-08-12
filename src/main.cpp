#include <QApplication>
#include "aradomain.h"

int
main (int argc, char *argv[])
{
  QApplication app (argc, argv);

  arado::AradoMain  arado (0,&app);

  arado.Start ();

  int appresult = app.exec ();

  return appresult + 42;
}
