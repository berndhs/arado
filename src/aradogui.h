#ifndef ARADOGUI_H
#define ARADOGUI_H

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
#include "ui_aradomain.h"
#include "db-manager.h"

#include <QMainWindow>
#include <QUuid>

class QCloseEvent;
class QApplication;
class QTimer;

namespace arado
{

class AddRssFeed;
class ConfigEdit;
class FileComm;
class UrlDisplay;
class ConnectionDisplay;
class EntryForm;
class AradoUrl;
class Policy;
class HttpServer;
class HttpClient;
class AddPeerDialog;
class ListenerEdit;
class PollSequence;
class RssList;
class RssPoll;
class UPnPClient;
class CrawlerSettings;

class AradoMain : public QMainWindow
{
Q_OBJECT

public:

  AradoMain (QWidget *parent, QApplication * pa);

  void Start ();

  void closeEvent (QCloseEvent * event);

  bool Again ();

public slots:

  void Quit ();
  void slotAbout();
  void DoConfigEdit ();
  void DoneConfigEdit (bool saved);
  void DoneAddFeed (bool changed);
  void DoFileImport ();
  void DoFileExport ();
  void DoIpImport ();
  void DoIpExport ();
  void DoEntry (QString urlText = QString());
  void DoneEntry ();
  void NewUrl (const AradoUrl & newurl);

private slots:

  void AddServer ();
  void AddFeed ();
  void RefreshPeers ();
  void PeersAdded (int howmany = 1);
  void UrlsAdded (int howmany);
  void HaveNewPeer (QString peerid);
  void Poll (bool haveNew=false);
  void PollNow (bool haveNew);
  void EditListener ();
  void MailUuid ();
  void DisplayUuid ();
  void InitSystem ();
  void CatchPolledRss (QString nick);
  void Crawl ();

  void Restart ();

private:

  void  Connect ();
  void  StartServers ();
  void  StartClients ();
  void  StopServers ();
  void  StopClients ();
  void  StartSequencer ();
  void  StopSequencer ();

  Ui_AradoWin         mainUi;
  QApplication       *app;
  bool                setupDone;
  ConfigEdit         *configEdit;
  FileComm           *fileComm;
  UrlDisplay         *urlDisplay;
  ConnectionDisplay  *connDisplay;
  EntryForm          *entryForm;
  AddPeerDialog      *addPeerDialog;
  ListenerEdit       *listenerEdit;
  CrawlerSettings    *crawler;
  DBManager           dbMgr;
  Policy             *policy;
  PollSequence       *sequencer;
  RssList            *rssList;
  RssPoll            *rssPoll;
  HttpServer         *httpServer;
  HttpClient         *httpClient;
  QTimer             *httpPoll;
  int                 httpDefaultPort;
  QUuid               ownUuid;
  bool                runAgain;
#if USE_MINIUPNP
  UPnPClient *uPnPClient;
#endif

};

} // namespace

#endif