#include "aradomain.h"
#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include "arado-url.h"
#include "file-comm.h"
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include "deliberate.h"
#include "version.h"
#include "config-edit.h"
#include "url-display.h"
#include "connection-display.h"

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Arado Team
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
 *  Boston, MA  02110-1301, USA.
 ****************************************************************/

namespace arado
{

AradoMain::AradoMain (QWidget *parent, QApplication *pa)
  :QMainWindow (parent),
   app (0),
   setupDone (false),
   configEdit (0),
   fileComm (0),
   urlDisplay (0),
   connDisplay (0),
   dbMgr (this)
{
  app = pa;
  mainUi.setupUi (this);
  configEdit = new ConfigEdit (this);
  urlDisplay = new UrlDisplay (this);
  urlDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (urlDisplay, tr("Database"));
  connDisplay = new ConnectionDisplay (this);
  connDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (connDisplay, tr("Connections"));
  refreshUrls = new QTimer (this);
}

/// \brief Start the main window, initialize

void
AradoMain::Start ()
{
  if (!setupDone) {
    Connect ();
    dbMgr.Start ();
    if (refreshUrls) {
      refreshUrls->start (15000);
    }
    setupDone = true;
    if (urlDisplay) {
      urlDisplay->Refresh();
    }
  }
  show ();
}

void
AradoMain::Connect ()
{
  connect (mainUi.actionExit, SIGNAL (triggered ()), this, SLOT (Quit ()));
  connect (mainUi.actionAbout, SIGNAL(triggered ()), this,
	  SLOT(slotAbout ()));
  connect (mainUi.actionSettings, SIGNAL (triggered ()), this,
          SLOT (DoConfigEdit()));
  connect (mainUi.actionFileComm, SIGNAL (triggered ()),
          this, SLOT (DoFileComm ()));
  if (configEdit) {
    connect (configEdit, SIGNAL (Finished(bool)), 
             this, SLOT (DoneConfigEdit (bool)));
  }
  if (refreshUrls && urlDisplay) {
    connect (refreshUrls, SIGNAL (timeout()), 
             urlDisplay, SLOT (Refresh()));
  }
}

/// \brief Close down

void
AradoMain::Quit ()
{
  dbMgr.Close ();
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


//
// ABOUT MESSAGE WINDOW
//
//
void
AradoMain::slotAbout ()
{
  QMessageBox mb(this);

  mb.setWindowTitle(tr("ARADO - URL Database"));
  mb.setTextFormat(Qt::RichText);
  QString text = QString("<html>"
             "%1"
             "<br><hr>Open Source software for Linux, Mac, and Windows.<br><br>"
	     "<hr>"
             "Please visit <a href='http://arado.sf.net'>"
             "http://arado.sf.net</a><br>for more information."
             "</html>")
             .arg (deliberate::ProgramVersion::Version());
  mb.setText(text);
  mb.setStandardButtons(QMessageBox::Ok);
  mb.setIconPixmap(QPixmap(":/arado-logo-colo-128.png"));
  mb.exec();
}

void
AradoMain::DoConfigEdit ()
{
  if (configEdit) {
    int tabnum = mainUi.tabWidget->addTab (configEdit, tr("Settings"));
    mainUi.tabWidget->setCurrentIndex (tabnum);
    configEdit->Run ();
  }
}

void
AradoMain::DoneConfigEdit (bool saved)
{
  if (configEdit) {
    int tabnum = mainUi.tabWidget->indexOf (configEdit);
    mainUi.tabWidget->removeTab (tabnum);
    if (saved) {
      // reload settings 
      qDebug () << " Settings editor saved, should reload settings";
    }
  }
}

void
AradoMain::DoFileComm ()
{
  QString filename = QFileDialog::getOpenFileName (this);
qDebug () << " they want file " << filename;
  if (filename.length() <= 0) {
    return;
  }
  if (!fileComm) {
    fileComm = new FileComm;
  }
  AradoUrlList  urlist;
  if (fileComm) {
    fileComm->Open (filename, QFile::ReadOnly);
    urlist = fileComm->Read ();
    fileComm->Close ();
    for (int u=0; u<urlist.size(); u++) {
      dbMgr.AddUrl (urlist[u]);
    }
    QString savehere = QFileDialog::getSaveFileName (this);
    if (savehere.length() > 0) {
      fileComm->Open ("out.xml",QFile::WriteOnly);
      fileComm->Write (urlist);
      fileComm->Close ();
    }
  }
  
}


} // namespace






