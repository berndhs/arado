
/** \brief HttpSender is the worker thread for HttpServer
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

#include "http-sender.h"
#include "db-manager.h"
#include <QTcpSocket>
#include <QByteArray>

namespace arado
{

HttpSender::HttpSender (int sock, QObject *parent, DBManager *dbm)
  :QThread (parent),
   socket (sock),
   db (dbm)
{ 
}

void
HttpSender::run ()
{
  qDebug () << " Sender Thread running";
  QTcpSocket tcpSocket;
  bool works = tcpSocket.setSocketDescriptor (socket);
  if (!works) {
    return;
  }
  qDebug () << " Sender Thread has good socket";
  QByteArray msgBytes = tcpSocket.read (1024);
  qDebug () << " Sender Thread incoming message " << msgBytes;
  qDebug () << " Sender Thread done";
}

} // namespace

