
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
#include <QHostInfo>

using namespace deliberate;

namespace arado
{

HttpServer::HttpServer (QObject *parent)
  :QTcpServer (parent),
   serverAddrString ("localhost"),
   serverPort (80),
   runServer (true),
   running (false),
   db (0),
   acceptCleaner (this),
   acceptPause (15),
   grantGet (true),
   allowPut (true),
   tradeAddr (true),
   tradeUrl (true)
{
  connect (&acceptCleaner, SIGNAL (timeout()), this, SLOT (CleanAccept()));
  grantGet = Settings().simpleValue ("http/getgrant",grantGet).toBool();
  Settings ().setSimpleValue ("http/getgrant",grantGet);
  allowPut = Settings().simpleValue("http/putallow",allowPut).toBool();
  Settings ().setSimpleValue ("http/putallow",allowPut);
  tradeAddr = Settings().simpleValue ("transfer/addresses",tradeAddr).toBool();
  Settings().setSimpleValue ("transfer/addresses", tradeAddr);
  tradeUrl = Settings().simpleValue ("transfer/urls",tradeUrl).toBool();
  Settings().setSimpleValue ("transfer/urls", tradeUrl);
}

bool
HttpServer::Start ()
{
  runServer = Settings().simpleValue ("http/run",runServer).toBool ();
  Settings().setSimpleValue ("http/run",runServer);
  serverAddrString = Settings().simpleValue ("http/address",serverAddrString).toString();
  Settings().setSimpleValue ("http/address",serverAddrString);
  serverPort = Settings().simpleValue ("http/port",serverPort).toUInt();
  Settings().setSimpleValue ("http/port",serverPort);
  acceptPause = Settings().simpleValue ("http/coolingtime",acceptPause).toInt();
  Settings().setSimpleValue ("http/coolingtime",acceptPause);
  if (runServer) {
    Listen (serverAddrString, serverPort);
  }
  acceptCleaner.start (acceptPause);
  return false;
}

bool
HttpServer::Stop ()
{
  acceptCleaner.stop ();
  close ();
  return true;
}

bool
HttpServer::Listen (const QHostAddress & address, 
                          quint16 port)
{
  bool ok = listen (address, port);
  qDebug () << " listen " << ok << " address " << address << " port " << port;
  running |= ok;
  return ok;
}

bool
HttpServer::Listen (const QString & host, quint16 port)
{
qDebug () << " try listen to host " << host << " port " << port;
  if (deliberate::IsIp6Address (host) || deliberate::IsIp4Address (host)) {
    return Listen (QHostAddress (host), port);
  } else {
    QHostInfo hinfo = QHostInfo::fromName (host);
qDebug () << "listener " << host << "  address list " << hinfo.addresses();
    QList<QHostAddress> addrList = hinfo.addresses();
    QHostAddress addr;
    if (!addrList.isEmpty()) {
      addr = hinfo.addresses().first ();
      return Listen (addr, port);
    }
  }
  return false;
}

void
HttpServer::incomingConnection (int sock)
{
  HttpSender * sender = new HttpSender (sock, this, db, policy, 
                                        grantGet, allowPut, tradeAddr, tradeUrl,
                                        expectPeer, expectData, lastAccept);
  connect (sender, SIGNAL (finished()), sender, SLOT (deleteLater ()));
  connect (sender, SIGNAL (ExpectData (QString, QString, QString)),
           this, SLOT (MarkExpected (QString, QString, QString)));
  connect (sender, SIGNAL (ReceivingData (QString)),
           this, SLOT (MarkReceiving (QString)));
  connect (sender, SIGNAL (AddedPeers (int)), this, SLOT (PeersAdded (int)));
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

void
HttpServer::PeersAdded (int howmany)
{
  emit AddedPeers (howmany);
}

} // namespace

