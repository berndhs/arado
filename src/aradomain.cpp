#include "aradomain.h"
#include <QCloseEvent>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFileInfo>
#include <QSplashScreen>
#include <QTimer>
#include "arado-url.h"
#include "file-comm.h"
#include <QDebug>
#include <QUrl>
#include <QTimer>
#include <QStyle>
#include <QSound>
#include "deliberate.h"
#include "version.h"
#include "config-edit.h"
#include "url-display.h"
#include "connection-display.h"
#include "entry-form.h"
#include "add-peer.h"
#include "policy.h"
#include "poll-sequence.h"
#include "search.h"
#include "http-server.h"
#include "http-client.h"
#include "ui_feed-input.h"
#include "arado-stream-parser.h"
#include "listener-edit.h"
#include "addfeed.h"
#include "rss-list.h"
#include "rss-poll.h"

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

#include <QDialog>

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
   addPeerDialog (0),
   listenerEdit (0),
   dbMgr (this),
   policy (0),
   sequencer (0),
   rssList (0),
   rssPoll (0),
   httpServer (0),
   httpClient (0),
   httpPoll (0),
   httpDefaultPort (29998),
   ownUuid (QUuid()),
   runAgain (false)
{
  app = pa;
  mainUi.setupUi (this);
  configEdit = new ConfigEdit (this);
  urlDisplay = new UrlDisplay (this);
  urlDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (urlDisplay, tr("Websearch"));
  entryForm = new EntryForm (this);
  entryForm->SetDB (&dbMgr);
  addPeerDialog = new AddPeerDialog (this);
  addPeerDialog->SetDB (&dbMgr);
  listenerEdit = new ListenerEdit (this);
  policy = new Policy (this);
  sequencer = new PollSequence (this);
  sequencer->SetDB (&dbMgr);
  rssList = new RssList (this);
  rssList->SetDB (&dbMgr);
  rssPoll = new RssPoll (this);
  rssPoll->SetDB (&dbMgr);
  httpServer = new HttpServer (this);
  httpClient = new HttpClient (this);
  httpPoll = new QTimer (this);
  connDisplay = new ConnectionDisplay (this);
  connDisplay->SetDB (&dbMgr);
  mainUi.tabWidget->addTab (connDisplay, tr("Network"));
  mainUi.tabWidget->addTab (entryForm, tr("Add URL"));

}

/// \brief Start the main window, initialize

void
AradoMain::Start ()
{
  runAgain = false;
  qDebug () << "Arado Starting " << QDateTime::currentDateTime();
  if (!setupDone) {
    if (Settings().contains("sizes/main")) {
      QSize defaultSize = size ();
      QSize newSize = Settings().value ("sizes/main", defaultSize).toSize();
      resize (newSize);
    }
    ownUuid = QUuid::createUuid();
    ownUuid = QUuid (Settings().value ("personal/uuid",
                     ownUuid.toString()).toString());
    Settings().setValue ("personal/uuid",ownUuid.toString());
    Connect ();
    dbMgr.Start ();
    setupDone = true;
    if (urlDisplay) {
      QTimer::singleShot (1200, urlDisplay, SLOT (Refresh()));
    }
    if (policy) {
      policy->Load (&dbMgr);
    }
    if (connDisplay) {
      connDisplay->Start ();
    }
  }
  show ();
  StartServers ();
  StartClients ();
  RefreshPeers ();
  StartSequencer ();
  if (rssPoll) {
    rssPoll->Start ();
  }
  if (dbMgr.NumPeers () < 1) {
    QTimer::singleShot (5000,this,SLOT (InitSystem()));
  }
}

bool
AradoMain::Again ()
{
  bool again = runAgain;
  runAgain = false;
  return again;
}

void
AradoMain::Restart ()
{
  runAgain = true;
  StopServers ();
  StopClients ();
  if (rssPoll) {
    rssPoll->Stop ();
  }
  Quit ();
}

void
AradoMain::StartServers ()
{
  if (httpServer) {
    httpServer->SetDB (&dbMgr);
    httpServer->SetPolicy (policy);
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
             this, SLOT (Poll()));
    httpPoll->start (2*60*1000);
    httpClient->SetDB (&dbMgr);
    httpClient->SetPolicy (policy);
    QTimer::singleShot (3000, this, SLOT (Poll()));
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
AradoMain::StartSequencer ()
{
  if (sequencer) {
    sequencer->Start ();
  }
}

void
AradoMain::StopSequencer ()
{
  if (sequencer) {
    sequencer->Stop ();
  }
}

void
AradoMain::AddServer ()
{
  addPeerDialog->Run ();
}

void
AradoMain::AddFeed ()
{
  if (rssList) {
    mainUi.tabWidget->addTab (rssList, tr("Rss Feeds"));
    rssList->Show();
  }
}

void
AradoMain::DoneAddFeed (bool changed)
{
  if (rssList) {
    int tabNum = mainUi.tabWidget->indexOf (rssList);
    mainUi.tabWidget->removeTab (tabNum);
  }
  if (rssPoll && changed) {
    rssPoll->Stop ();
    rssPoll->Start ();
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
  connect (mainUi.actionAddServer, SIGNAL (triggered()),
           this, SLOT (AddServer ()));
  connect (mainUi.actionPoll, SIGNAL (triggered()),
           this, SLOT (Poll ()));
  connect (mainUi.actionAddFeed, SIGNAL (triggered()),
           this, SLOT (AddFeed ()));
  connect (mainUi.actionExportIP, SIGNAL (triggered()),
           this, SLOT (DoIpExport ()));
  connect (mainUi.actionImportIP, SIGNAL (triggered()),
           this, SLOT (DoIpImport ()));
  connect (mainUi.actionListener, SIGNAL (triggered()),
           this, SLOT (EditListener ()));
  connect (mainUi.actionMailUuid, SIGNAL (triggered()),
           this, SLOT (MailUuid ()));
  connect (mainUi.actionShowUuid, SIGNAL (triggered()),
           this, SLOT (DisplayUuid ()));
  connect (mainUi.actionRestart, SIGNAL (triggered()),
           this, SLOT (Restart ()));
  connect (mainUi.actionInitialize, SIGNAL (triggered()),
           this, SLOT (InitSystem()));
  connect (addPeerDialog, 
             SIGNAL (NewPeer (QString)),
           this, 
             SLOT (HaveNewPeer (QString)));
  connect (connDisplay,
             SIGNAL (HaveNewPeer ()),
           this,
             SLOT (RefreshPeers ()));
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
      connect (httpClient, SIGNAL (AddedPeers (int)),
               this, SLOT (PeersAdded (int)));
    }
  }
  if (httpServer) {
    connect (httpServer, SIGNAL (AddedPeers (int)),
             this, SLOT (PeersAdded (int)));
  }
  if (connDisplay) {
    connect (connDisplay, SIGNAL (WantEditListener()),
            this, SLOT (EditListener()));
    connect (connDisplay, SIGNAL (AddDevice()), this, SLOT (AddServer()));
    connect (connDisplay, SIGNAL (StartSync(bool)), this, SLOT (PollNow(bool)));
    if (sequencer) {
      connect (connDisplay, SIGNAL (TrafficParamsChanged ()),
               sequencer, SLOT (Restart()));
    }
  }
  if (httpClient && httpPoll) {
    connect (httpPoll, SIGNAL (timeout()), this, SLOT (Poll()));
  }
  if (listenerEdit) {
    connect (listenerEdit, SIGNAL (SuggestRestart()),
            this, SLOT (Restart()));
  }
  if (rssList) {
    connect (rssList, SIGNAL (Closed(bool)),
             this, SLOT (DoneAddFeed (bool)));
  }
  if (rssPoll) {
    connect (rssPoll, SIGNAL (PolledRss (QString)),
             this, SLOT (PolledRss (QString)));
  }
}

/// \brief Close down

void
AradoMain::Quit ()
{
  QSize currentSize = size();
  Settings().setValue ("sizes/main",currentSize);
  Settings().sync();
  dbMgr.Stop ();
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

  mb.setWindowTitle(tr("Arado - Websearch"));
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
  mb.setStyleSheet( "background-color: qlineargradient(spread:pad, x1:0.968, y1:0.932, x2:0.513, y2:0.5, stop:0 rgba(183, 235, 255, 255), stop:1 rgba(255, 255, 255, 255));" );
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
      dbMgr.Stop  ();
      dbMgr.Start ();
      if (policy) {
        policy->Flush ();
        policy->Load (&dbMgr);
      }
      StopSequencer ();
      StopServers ();
      StopClients ();
      if (rssPoll) {
        rssPoll->Stop();
      }
      StartServers ();
      StartClients ();
      StartSequencer ();
      if (rssPoll) {
        rssPoll->Start ();
      }
    }
  }
}

void
AradoMain::DoEntry (QString urlText)
{
  if (entryForm) {
    int tabnum = mainUi.tabWidget->addTab (entryForm,tr("Add URL"));
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
    Q_UNUSED (newurl)
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
                     tr("Export URL File"),
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

void
AradoMain::DoIpImport ()
{
  static QString dir ( QDesktopServices::storageLocation
                    (QDesktopServices::HomeLocation));
  QString filename = QFileDialog::getOpenFileName (this,
                     tr("Import Address File"),
                     dir,
                     tr ("Arado Address Files (*.xml);; All Files (*)")
                     );
  if (filename.length() <= 0) {
    return;
  }
  QFile file (filename);
  bool ok = file.open (QFile::ReadOnly);
  if (ok) {
    AradoStreamParser parser;
    parser.SetInDevice (&file);
    AradoPeerList list = parser.ReadAradoPeerList ();
    file.close ();
    AradoPeerList::iterator it;
    quint64 seq = QDateTime::currentDateTime().toTime_t();
    seq *= 10000;
    QString elvis ("Elvis");
    elvis = Settings().value ("personal/elvis",elvis).toString();
    Settings().setValue ("personal/elvis",elvis);
    int e (1);
    for (it=list.begin(); it != list.end(); it++, e++) {
      it->SetNick (QString ("%1%2").arg(elvis).arg(seq+e));
      dbMgr.AddPeer (*it);
    }
    RefreshPeers ();
    if (connDisplay) {
      connDisplay->ShowPeers ();
    }
  }
  
}

void
AradoMain::DoIpExport ()
{
  static QString dir ( QDesktopServices::storageLocation
                    (QDesktopServices::HomeLocation));
  QString savehere = QFileDialog::getSaveFileName (this,
                     tr("Export Address File"),
                     dir,
                     tr ("Arado Address Files (*.xml);; All Files (*)")
                     );
  dir = QFileInfo (savehere).absolutePath ();
  if (savehere.length() > 0) {
    if (!QFileInfo (savehere).exists()) {
      QDir d (dir);
      d.mkpath (dir);
      QFile  file (savehere);
      file.open (QFile::WriteOnly);
      AradoPeerList  list = dbMgr.GetPeers ("A");
      list += dbMgr.GetPeers ("B");
      list += dbMgr.GetPeers ("C");
      AradoStreamParser  parser;
      parser.SetOutDevice (&file);
      parser.Write (list);
      file.close ();
    }
  }
}

void
AradoMain::RefreshPeers ()
{
  if (httpClient) {
    httpClient->DropAllServers ();
    httpClient->ReloadServers ("A");
    httpClient->ReloadServers ("B");
    httpClient->ReloadServers ("C");
  }
}

void
AradoMain::HaveNewPeer (QString peerid)
{
  Q_UNUSED (peerid)
  PeersAdded ();
}

void
AradoMain::PeersAdded (int howmany)
{
  if (howmany > 0) {
    RefreshPeers ();
    if (connDisplay) {
      connDisplay->ShowPeers ();
    }
  }
}

void
AradoMain::UrlsAdded (int howmany)
{
  if (howmany > 0) {
    if (urlDisplay) {
      urlDisplay->Refresh ();
    }
  }
}

void
AradoMain::Poll (bool haveNew)
{
  Q_UNUSED (haveNew)
  if (sequencer && httpClient) {
    sequencer->PollClient (httpClient);
  }
}

void
AradoMain::PollNow (bool haveNew)
{
  if (sequencer && httpClient) {
    sequencer->PollClient (httpClient, haveNew);
  }
}

void
AradoMain::MailUuid ()
{

  //
  // Define Desktop-Mail-Client-Url: mailUrl
  //

  QString welcomeline (tr("Please join me at Arado."));
  QString blankline (tr(""));
  QString uuline (tr("Add my Arado-UUID: %1").arg (ownUuid));
  QString webpageline (tr("http://arado.sf.net Websearch - Syncs, shortens "
                          "and searches within (y)our URLs and Bookmarks."));
  QUrl   mailUrl;
  mailUrl.setScheme ("mailto");
  mailUrl.addQueryItem ("subject",tr("My Arado UUID"));
  QStringList mailBody;
  mailBody << welcomeline
           << blankline
           << uuline
           << blankline
           << webpageline ;
  mailUrl.addQueryItem ("body",mailBody.join("\r\n"));
  QDesktopServices::openUrl (mailUrl);
}

void
AradoMain::DisplayUuid ()
{
  QMessageBox  box (this);
  box.setWindowTitle (tr("Arado"));
  box.setIconPixmap(QPixmap(":/images/roll.png"));
  box.setText (tr ("Your Arado UUID is \n\n"
                        "%1")
               .arg (ownUuid.toString()));
  QPushButton * copyBut = box.addButton (tr("Copy UUID"), 
                           QMessageBox::YesRole);
  box.addButton (tr("OK"),
                 QMessageBox::AcceptRole);
  QTimer::singleShot (30000, &box, SLOT (accept()));
  box.exec ();
  if (box.clickedButton() == copyBut) {
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText (ownUuid.toString ());
  }
}

void
AradoMain::EditListener ()
{
  if (listenerEdit) {
    listenerEdit->Run ();
  }
}

void
AradoMain::InitSystem ()
{
  AradoPeer initPeer;
  initPeer.SetNick ("AradoElvis_1288111461");
  initPeer.SetAddr ("arado.sourceforge.net");
  initPeer.SetAddrType ("name");
  initPeer.SetPort (80);
  initPeer.SetLevel ("C");
  dbMgr.AddPeer (initPeer);
  PeersAdded (1);
  PollNow (true);
}

void
AradoMain::PolledRss (QString nick)
{
  if (nick.length() > 0) {
    QDateTime now = QDateTime::currentDateTime();
    urlDisplay->SetMiddleMessage (tr("%1: Polled RSS \"%2\"")
                      .arg(now.toString(tr ("hh:mm:ss")))
                      .arg(nick));  
  } else {
    urlDisplay->SetMiddleMessage ("");
  }
}

} // namespace






