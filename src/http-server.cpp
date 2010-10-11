
/** \brief HTTP Server for Arado
  */

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

#include "http-server.h"
#include "deliberate.h"
#include "db-manager.h"
#include "http-sender.h"
#include "policy.h"
#include <QTcpServer>
#include <QDateTime>

using namespace deliberate;

namespace arado
{

HttpServer::HttpServer (QObject *parent)
  :QTcpServer (parent),
   serverAddrString ("localhost"),
   serverPort (80),
   runServer (false),
   running (false),
   db (0),
   acceptCleaner (this),
   acceptPause (15),
   grantGet (true),
   allowPut (true),
   tradeAddr (true)
{
  connect (&acceptCleaner, SIGNAL (timeout()), this, SLOT (CleanAccept()));
  grantGet = Settings().value ("http/getgrant",grantGet).toBool();
  Settings ().setValue ("http/getgrant",grantGet);
  allowPut = Settings().value("http/putallow",allowPut).toBool();
  Settings ().setValue ("http/putallow",allowPut);
  tradeAddr = Settings().value ("trade/addresses",tradeAddr).toBool();
  Settings().setValue ("trade/addresses", tradeAddr);
}

bool
HttpServer::Start ()
{
  runServer = Settings().value ("http/run",runServer).toBool ();
  Settings().setValue ("http/run",runServer);
  serverAddrString = Settings().value ("http/address",serverAddrString).toString();
  Settings().setValue ("http/address",serverAddrString);
  serverPort = Settings().value ("http/port",serverPort).toUInt();
  Settings().setValue ("http/port",serverPort);
  acceptPause = Settings().value ("http/coolingtime",acceptPause).toInt();
  Settings().setValue ("http/coolingtime",acceptPause);
  if (runServer) {
    Listen (QHostAddress (serverAddrString), serverPort);
  }
  acceptCleaner.start (acceptPause);
  return false;
}

bool
HttpServer::Stop ()
{
  acceptCleaner.stop ();
  return true;
}

bool
HttpServer::Listen (const QHostAddress & address, 
                          quint16 port)
{
  bool ok = listen (address, port);
  running |= ok;
  return ok;
}

void
HttpServer::incomingConnection (int sock)
{
  HttpSender * sender = new HttpSender (sock, this, db, policy, 
                                        grantGet, allowPut, tradeAddr,
                                        expectPeer, expectData, lastAccept);
  connect (sender, SIGNAL (finished()), sender, SLOT (deleteLater ()));
  connect (sender, SIGNAL (ExpectData (QString, QString, QString)),
           this, SLOT (MarkExpected (QString, QString, QString)));
  connect (sender, SIGNAL (ReceivingData (QString)),
           this, SLOT (MarkReceiving (QString)));
  sender->start ();
}

void
HttpServer::MarkExpected (QString path, QString peer, QString datatype)
{
  expectPeer [path] = peer;
  expectData [path] = datatype;
  quint64 now = QDateTime::currentDateTime().toTime_t();
  lastAccept [peer] = now;
  qDebug () << " inserted uupath " << path;
}

void
HttpServer::MarkReceiving (QString path)
{
  expectPeer.remove (path);
  expectData.remove (path);
  qDebug () << " removed uupath " << path;
}

void
HttpServer::CleanAccept ()
{
  quint64 limit = QDateTime::currentDateTime().toTime_t();
  limit -= acceptPause;
  QMap <QString, quint64>::iterator  chase = lastAccept.begin();
  while (chase != lastAccept.end() ) {
    if (*chase < limit) {
      chase = lastAccept.erase (chase);
    } else {
      chase ++;
    }
  }
}


} // namespace

