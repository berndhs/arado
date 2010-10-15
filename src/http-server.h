#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

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

#include <QTcpServer>
#include <QHostAddress>
#include <QMap>
#include <QString>
#include <QTimer>

namespace arado
{

class DBManager;
class Policy;

class HttpServer : public QTcpServer
{
Q_OBJECT

public:

  HttpServer (QObject *parent=0);

  void SetDB (DBManager *dbm) { db = dbm; }
  void SetPolicy (Policy *pol) { policy = pol; }

  bool Start ();
  bool Stop ();
  bool Running () { return running; }
  bool Listen ( const QString & host, quint16 port = 0);
  bool Listen ( const QHostAddress & address = QHostAddress::Any, 
                quint16 port = 0 );

private slots:

  void  MarkExpected (QString path, QString host, QString datatype);
  void  MarkReceiving (QString path);
  void  CleanAccept ();
  void  PeersAdded (int howmany = 1);

signals:

  void AddedPeers (int howmany);

protected:

  void incomingConnection(int sock);

private:


  QString        serverAddrString;
  quint16        serverPort;
  bool           runServer;
  bool           running;
  DBManager     *db;
  Policy        *policy;

  QMap <QString, QString>  expectPeer;
  QMap <QString, QString>  expectData;
  QMap <QString, quint64>  lastAccept;

  QTimer        acceptCleaner;
  quint64       acceptPause;
  bool          grantGet;
  bool          allowPut;
  bool          tradeAddr;
  bool          tradeUrl;

};

} // namespace

#endif

