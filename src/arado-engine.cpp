#include "arado-engine.h"


/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2011, Arado Team
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

#include <QTimer>
#include <QFile>
#include <QLocalSocket>
#include <QStringList>
#include <QByteArray>
#include <QString>
#include <QDebug>

#include "policy.h"
#include "rss-poll.h"
#include "http-server.h"
#include "http-client.h"
#include "poll-sequence.h"

namespace arado
{

AradoEngine::AradoEngine (QObject *parent)
  :QLocalServer (parent),
   app (0),
   mainPipe (0),
   dbMgr (this),
   policy (0),
   rssPoll (0),
   httpDefaultPort (29998),
   sequencer (0),
   httpServer (0),
   httpClient (0),
   httpPoll (0)
{
  policy = new Policy (this);
  rssPoll = new RssPoll (this);
  rssPoll->SetDB (&dbMgr);
  connect (rssPoll, SIGNAL (SigPolledRss (QString)),
          this, SLOT (ReportRssPoll (QString)));
  dbMgr.SetPolicy (policy);
  sequencer = new PollSequence (this);
  sequencer->SetDB (&dbMgr);
  httpServer = new HttpServer (this);
  httpClient = new HttpClient (this);
  httpPoll = new QTimer (this);
  ConnectSignals ();
}

AradoEngine::~AradoEngine ()
{
}

void
AradoEngine::Start (const QString & serviceId)
{
  qDebug () << " AradoEngine Start";
  serviceName = QString ("aradoen%1").arg(serviceId);
  qDebug () << " AradoEngine service name " << serviceName;
}

void
AradoEngine::StartServer ()
{
  qDebug () << " AradoEngine StartServer";
  bool ok = listen (serviceName);
  if (!ok) {
    Bailout ("AradoEngine exiting: cannot listen to pipe");
    return;
  }
  bool conOk = connect (this, SIGNAL (newConnection()), this, 
                     SLOT (GetNewConnection()));
  qDebug () << " AradoEngine listening on " << serverName() 
            << " conOk " << conOk;
  dbMgr.Start ();
  if (policy) {
    policy->Load (&dbMgr);
  }
  StartHttpServers ();
  StartHttpClients ();
  RefreshPeers ();
  StartSequencer ();
  if (rssPoll) {
    rssPoll->Start ();
  }
}

void
AradoEngine::StartHttpServers ()
{
  if (httpServer) {
    httpServer->SetDB (&dbMgr);
    httpServer->SetPolicy (policy);
    httpServer->Start ();
  }
}

void
AradoEngine::StopHttpServers ()
{
  if (httpServer) {
    httpServer->Stop ();
  }
}

void
AradoEngine::StartHttpClients ()
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
AradoEngine::StopHttpClients ()
{
  if (httpPoll) {
    httpPoll->stop ();
  }
  if (httpClient) {
    httpClient->DropAllServers ();
  }
}

void
AradoEngine::StartSequencer ()
{
  if (sequencer) {
    sequencer->Start ();
  }
}

void
AradoEngine::StopSequencer ()
{
  if (sequencer) {
    sequencer->Stop ();
  }
}

void
AradoEngine::ConnectSignals ()
{
  if (httpServer) {
    connect (httpServer, SIGNAL (AddedPeers (int)),
             this, SLOT (PeersAdded (int)));
  }
  if (httpClient && httpPoll) {
    connect (httpPoll, SIGNAL (timeout()), this, SLOT (Poll()));
  }
  if (httpClient) {
    connect (httpClient, SIGNAL (AddedUrls (int)),
             this, SLOT (UrlsAdded (int)));
    connect (httpClient, SIGNAL (AddedPeers (int)),
             this, SLOT (PeersAdded (int)));
  }
}

void
AradoEngine::UrlsAdded (int numNew)
{
  if (mainPipe) {
    mainPipe->write (QString("ADDEDURLS %1\n").arg(numNew).toAscii());
  }
}

void
AradoEngine::PeersAdded (int numNew)
{
  if (mainPipe) {
    mainPipe->write (QString("ADDEDPEERS %1\n").arg(numNew).toAscii());
  }
}

void
AradoEngine::RefreshPeers ()
{
  if (httpClient) {
    httpClient->DropAllServers ();
    httpClient->ReloadServers ("A");
    httpClient->ReloadServers ("B");
    httpClient->ReloadServers ("C");
  }
}
void
AradoEngine::Poll (bool haveNew)
{
  Q_UNUSED (haveNew)
  if (sequencer && httpClient) {
    sequencer->PollClient (httpClient);
  }
}

void
AradoEngine::GetNewConnection ()
{
  qDebug () << "AradoEngine New Connection for " << serverName();
  if (mainPipe) {
    qWarning ("AradoEngine: reject second connection");
    return;
  }
  mainPipe = nextPendingConnection ();
  if (mainPipe == 0) {
    qWarning ("AradoEngine: incoming connection not found");
    return;
  }
  connect (mainPipe, SIGNAL (disconnected()), this, SLOT (Disconnected()));
  connect (mainPipe, SIGNAL (readyRead()), this, SLOT (ReadPipe()));
  if (rssPoll) {
    rssPoll->Start ();
  }
  mainPipe->write (QByteArray ("RUNNING\n"));
}

void
AradoEngine::Disconnected ()
{
  QObject *sigSource = sender ();
  if (sigSource) {
    QLocalSocket * sock = dynamic_cast<QLocalSocket*> (sigSource);
    if (sock && sock == mainPipe) {
      QTimer::singleShot (100, this, SLOT(Quit ()));
    }
  }
}

void
AradoEngine::Bailout (const char * msg)
{
  qWarning (msg);
  Quit ();
}

void
AradoEngine::ReadPipe ()
{
  QByteArray data = mainPipe->readLine (16*1024);
  qDebug () << " AradoEngine reads " << data;
  if (data.startsWith(QByteArray ("CLOSE"))) {
    Quit ();
    return;
  }
  QStringList parts = QString(data).split (QRegExp("\\s"),
                                           QString::SkipEmptyParts);
  QString cmd = parts.at(0);
  qDebug () << " AradoEngine ReadPipe " << data;
  if (cmd.startsWith ("RSSRESTART")) {
    if (rssPoll) {
      rssPoll->Stop ();
      rssPoll->Start ();
    }
  }
}

void
AradoEngine::ReportRssPoll (QString nick)
{
qDebug () << " report rss poll " << nick << " to pipe " << mainPipe;
  if (mainPipe) {
    QString msgPat ("RSSPOLL %1\n");
    mainPipe->write (msgPat.arg(nick).toAscii());
    mainPipe->flush ();
  }
}

void
AradoEngine::Quit ()
{
  qDebug () << " AradoEngine quitting";
  close ();
  if (rssPoll) {
    rssPoll->Stop ();
  }
  if (mainPipe) {
    mainPipe->disconnect ();
    delete mainPipe;
    mainPipe = 0;
  }
  if (app) {
    app->quit ();
  }
}

} // namespace
