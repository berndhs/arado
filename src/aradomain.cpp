#include "aradomain.h"
#include <QCloseEvent>
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileInfo>
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
#include "entry-form.h"
#include "policy.h"
#include "search.h"
#include "http-server.h"
#include "http-client.h"

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

using namespace deliberate;

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
   entryForm (0),
   dbMgr (this),
   policy (0),
   httpServer (0),
   httpClient (0),
   httpPoll (0),
   httpDefaultPort (29998)
{
  app = pa;
  mainUi.setupUi (this);
  configEdit = new ConfigEdit (this);
  urlDisplay = new UrlDisplay (this);
  urlDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (urlDisplay, tr("Search"));
  connDisplay = new ConnectionDisplay (this);
  connDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (connDisplay, tr("Network"));
  entryForm = new EntryForm (this);
  entryForm->SetDB (&dbMgr);
  policy = new Policy (this);
  httpServer = new HttpServer (this);
  httpClient = new HttpClient (this);
  httpPoll = new QTimer (this);
}

/// \brief Start the main window, initialize

void
AradoMain::Start ()
{
  if (!setupDone) {
    if (Settings().contains("sizes/main")) {
      QSize defaultSize = size ();
      QSize newSize = Settings().value ("sizes/main", defaultSize).toSize();
      resize (newSize);
    }//
    Connect ();
    dbMgr.Start ();
    setupDone = true;
    if (urlDisplay) {
      QTimer::singleShot (1200, urlDisplay, SLOT (Refresh()));
    }
  }
  show ();
  StartServers ();
  StartClients ();
}

void
AradoMain::StartServers ()
{
  if (httpServer) {
    httpServer->SetDB (&dbMgr);
    httpServer->Start ();
  }
}

void
AradoMain::StopServers ()
{
  if (httpServer) {
    httpServer->Stop ();
  }
}

void
AradoMain::StartClients ()
{
  if (httpPoll && httpClient) {
    connect (httpPoll, SIGNAL (timeout()),
             httpClient, SLOT (Poll()));
    httpPoll->start (2*60*1000);
    httpClient->SetDB (&dbMgr);
    httpClient->SetPolicy (policy);
    httpClient->AddServer (
                  QHostAddress ("2001:4830:1135:1:250:baff:fe18:fce6"),
                  httpDefaultPort);
    httpClient->AddServer (
                  QHostAddress ("178.77.66.196")
                  ,80);
    httpClient->AddServer (QUrl ("http://barbados.reflective-computing.com"),
                  httpDefaultPort);
    QTimer::singleShot (3000, httpClient, SLOT (Poll()));
  }
}

void
AradoMain::StopClients ()
{
  if (httpPoll) {
    httpPoll->stop ();
  }
  if (httpClient) {
    httpClient->DropAllServers ();
  }
}

void
AradoMain::Connect ()
{
  connect (mainUi.actionExit, SIGNAL (triggered ()), this, SLOT (Quit ()));
  connect (mainUi.actionAbout, SIGNAL(triggered ()), this,
	  SLOT(slotAbout ()));
  connect (mainUi.actionSettings, SIGNAL (triggered ()), this,
          SLOT (DoConfigEdit()));
  connect (mainUi.actionImportFile, SIGNAL (triggered ()),
          this, SLOT (DoFileImport ()));
  connect (mainUi.actionExportFile, SIGNAL (triggered ()),
          this, SLOT (DoFileExport ()));
  connect (mainUi.actionEnterData, SIGNAL (triggered()),
           this, SLOT (DoEntry ()));
  if (configEdit) {
    connect (configEdit, SIGNAL (Finished(bool)), 
             this, SLOT (DoneConfigEdit (bool)));
  }
  if (entryForm) {
    connect (entryForm, SIGNAL (Finished ()), this, SLOT (DoneEntry()));
    connect (entryForm, SIGNAL (NewUrl (const AradoUrl &)),
             this, SLOT (NewUrl (const AradoUrl &)));
  }
  if (urlDisplay) {
    connect (urlDisplay, SIGNAL (AddUrl(QString)),
             this, SLOT (DoEntry (QString)));
    if (httpClient) {
      connect (httpClient, SIGNAL (AddedUrls (int)),
               urlDisplay, SLOT (UrlsAdded (int)));
    }
  }
  if (httpClient && httpPoll) {
    connect (httpPoll, SIGNAL (triggered()), httpClient, SLOT (Poll()));
  }
}

/// \brief Close down

void
AradoMain::Quit ()
{
  QSize currentSize = size();
  Settings().setValue ("sizes/main",currentSize);
  Settings().sync();
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

  mb.setWindowTitle(tr("Arado - URL Database"));
  mb.setTextFormat(Qt::RichText);
  QString text = QString("<html>"
             "%1"
             "<br><hr>Open Source software for Linux, Mac, and Windows.<br><br>"
	     "<hr>"
             "Please visit <a href='http://arado.sf.net'>"
             "http://arado.sf.net</a><br>for more information."
             "</html>")
             .arg (ProgramVersion::Version());
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
      dbMgr.Close ();
      dbMgr.Start ();
      StopServers ();
      StopClients ();
      StartServers ();
      StartClients ();
    }
  }
}

void
AradoMain::DoEntry (QString urlText)
{
  if (entryForm) {
    int tabnum = mainUi.tabWidget->addTab (entryForm,tr("Enter Data"));
    mainUi.tabWidget->setCurrentIndex (tabnum);
    entryForm->Start (urlText);
  }
}

void
AradoMain::DoneEntry ()
{
  if (entryForm) {
    int tabnum = mainUi.tabWidget->indexOf (entryForm);
    mainUi.tabWidget->removeTab (tabnum);
  }
}

void
AradoMain::NewUrl (const AradoUrl & newurl)
{
  if (urlDisplay) {
    Q_UNUSED (urlDisplay)
    //urlDisplay->InsertUrl (newurl);
  }
}

void
AradoMain::DoFileImport ()
{
  static QString dir ( QDesktopServices::storageLocation
                    (QDesktopServices::HomeLocation));
  QString filename = QFileDialog::getOpenFileName (this,
                     tr("Import File"),
                     dir,
                     tr ("Arado Url Files (*.xml);; All Files (*)")
                     );
  if (filename.length() <= 0) {
    return;
  }
  dir = QFileInfo (filename).absolutePath ();
  if (!fileComm) {
    fileComm = new FileComm;
  }
  AradoUrlList  urlist;
  if (fileComm && policy) {
    fileComm->Open (filename, QFile::ReadOnly);
    urlist = fileComm->Read ();
    fileComm->Close ();
    for (int u=0; u<urlist.size(); u++) {
      policy->AddUrl (dbMgr, urlist[u]);
    }
    if (urlDisplay) {
      urlDisplay->Refresh (true);
    }
  }
}

void
AradoMain::DoFileExport ()
{
  static QString dir ( QDesktopServices::storageLocation
                    (QDesktopServices::HomeLocation));
  QString savehere = QFileDialog::getSaveFileName (this,
                     tr("Export File"),
                     dir,
                     tr ("Arado Url Files (*.xml);; All Files (*)")
                     );
  dir = QFileInfo (savehere).absolutePath ();
  if (savehere.length() > 0) {
    if (!fileComm) {
      fileComm = new FileComm;
    }
    fileComm->Open (savehere,QFile::WriteOnly);
    /// \brief ToDo: select urls to export
    AradoUrlList urlist = dbMgr.GetRecent (10000);
    fileComm->Write (urlist);
    fileComm->Close ();
  }
}


} // namespace






