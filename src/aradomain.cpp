#include "aradomain.h"
#include <QCloseEvent>
#include <QApplication>

namespace arado
{

AradoMain::AradoMain (QWidget *parent, QApplication *pa)
  :QMainWindow (parent),
   app (0),
   setupDone (false)
{
  app = pa;
  mainUi.setupUi (this);
}

void
AradoMain::Start ()
{
  if (!setupDone) {
    Connect ();
  }
  show ();
}

void
AradoMain::Connect ()
{
  connect (mainUi.actionExit, SIGNAL (triggered()), this, SLOT (Quit ()));
}

void
AradoMain::Quit ()
{
  if (app) {
    app->quit ();
  }
  hide ();
}

void 
AradoMain::closeEvent(QCloseEvent *event)
{
  if (app) {
    app->quit(); 
  }
  if (event) {
    event->accept ();
  }
}


} // namespace

