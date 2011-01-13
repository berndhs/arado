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

namespace arado
{

AradoEngine::AradoEngine (QObject *parent)
  :QLocalServer (parent),
   app (0),
   stdinFromMain (0),
   mainPipe (0)
{
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
}

void
AradoEngine::GetNewConnection ()
{
  if (mainPipe) {
    qWarning ("AradoEngine: refect second connection");
    return;
  }
  mainPipe = nextPendingConnection ();
  if (mainPipe == 0) {
    qWarning ("AradoEngine: incoming connection not found");
    return;
  }
  connect (mainPipe, SIGNAL (disconnected()), this, SLOT (Disconnected()));
  connect (mainPipe, SIGNAL (readyRead()), this, SLOT (ReadPipe()));
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
  QByteArray data = mainPipe->read (16*1024);
  qDebug () << " AradoEngine reads " << data;
  if (data == QByteArray ("CLOSE\n")) {
    Quit ();
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
