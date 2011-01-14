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
#include <stdio.h>
#include <QDebug>

#include "policy.h"
#include "rss-poll.h"

namespace arado
{

AradoEngine::AradoEngine (QObject *parent)
  :QLocalServer (parent),
   app (0),
   stdinFromMain (0),
   mainPipe (0),
   dbm (this),
   policy (0),
   rssPoll (0)
{
  policy = new Policy (this);
  rssPoll = new RssPoll (this);
  rssPoll->SetDB (&dbm);
  connect (rssPoll, SIGNAL (SigPolledRss (QString)),
          this, SLOT (ReportRssPoll (QString)));
  dbm.SetPolicy (policy);
}

AradoEngine::~AradoEngine ()
{
}

void
AradoEngine::Start ()
{
  stdinFromMain = new QFile (this);
  stdinFromMain->open (stdin, QFile::ReadOnly);
  connect (this, SIGNAL (newConnection()), this, 
                     SLOT (GetNewConnection()));
  QTimer::singleShot (250, this, SLOT(StartServer ()));
}

void
AradoEngine::StartServer ()
{
  if (stdinFromMain == 0 || !stdinFromMain->isOpen()) {
    Bailout ("AradoEngine no stdin");
    return;
  }
  QString data = stdinFromMain->readLine (2048);
  qDebug () << " AradoEngine got Hello " << data;
  if (data != "HELLO\n") {
    Bailout ("AradoEngine exiting: bad initial message");
    return;
  }
  data = stdinFromMain->readLine (2048);
  stdinFromMain->close ();
  qDebug () << " AradoEngine got service message " << data;
  QStringList parts = QString(data).split (QRegExp("\\s+"));
  if (parts.at(0) == "SERVE" && parts.count () > 1) {
    QString serviceName = QString ("aradoen%1").arg(parts.at(1));
    bool ok = listen (serviceName);
    if (!ok) {
      Bailout ("AradoEngine exiting: cannot listen to pipe");
      return;
    }
  } else {
    Bailout ("AradoEngine exiting: stdin protocol failure");
    return;
  }
  dbm.Start ();
  if (policy) {
    policy->Load (&dbm);
  }
}

void
AradoEngine::GetNewConnection ()
{
  qDebug () << "AradoEngine new connection for " << serverName();
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
    mainPipe->write (msgPat.arg(nick).toUtf8());
    mainPipe->flush ();
  }
}

void
AradoEngine::Quit ()
{
  qDebug () << " AradoEngine quitting";
  if (stdinFromMain) {
    stdinFromMain->close();
    delete stdinFromMain;
    stdinFromMain = 0;
  }
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
