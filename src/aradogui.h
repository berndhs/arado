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
class QProcess;
class QLocalSocket;

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
class AddPeerDialog;
class ListenerEdit;
class RssList;
class UPnPClient;
class CrawlerSettings;

class AradoMain : public QMainWindow
{
Q_OBJECT

public:

  AradoMain (QWidget *parent, QApplication * pa);

  void Start ();
  void DebugOpts (bool showD, bool logD, const QString & logF);

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
  void TabChanged(int tab);
  void RefreshPeers ();
  void PeersAdded (int howmany = 1);
  void UrlsAdded (int howmany);
  void HaveNewPeer (QString peerid);
  void PollNow (bool haveNew);
  void EditListener ();
  void MailUuid ();
  void DisplayUuid ();
  void InitSystem ();
  void CatchPolledRss (QString nick);
  void InitEngine ();
  void TimedConnectEngine ();
  void EngineConnected ();
  void EngineDisconnected ();
  void ReadEnginePipe ();
  void SequenceRestart ();

  void Restart ();

private:

  void  Connect ();
  void  StartServers ();
  void  StartClients ();
  void  StartEngine ();
  void  StopServers ();
  void  StopClients ();
  void  StopEngine ();
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
  DBManager           dbMgr;
  Policy             *policy;
  RssList            *rssList;
  int                 httpDefaultPort;
  QUuid               ownUuid;
  bool                runAgain;

  QLocalSocket       *enginePipe;
  QString             engineService;

  bool                showDebug;
  bool                logDebug;
  QString             logFile;

#if USE_MINIUPNP
  UPnPClient *uPnPClient;
#endif

};

} // namespace

#endif
