#ifndef ARADOMAIN_H
#define ARADOMAIN_H

#include "ui_aradomain.h"

#include <QMainWindow>

class QCloseEvent;
class QApplication;

namespace arado
{

class AradoMain : public QMainWindow
{
Q_OBJECT

public:

  AradoMain (QWidget *parent, QApplication * pa);

  void Start ();

  void closeEvent (QCloseEvent * event);

public slots:

  void Quit ();


private:

  void slotAbout(void);
  void  Connect ();

  Ui_AradoWin     mainUi;
  QApplication   *app;
  bool            setupDone;

};

} // namespace

#endif
