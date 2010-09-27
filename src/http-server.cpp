
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
#include <QTcpServer>

using namespace deliberate;

namespace arado
{

HttpServer::HttpServer (QObject *parent)
  :QTcpServer (parent),
   serverAddrString ("localhost"),
   serverPort (80),
   runServer (false),
   running (false),
   db (0)
{
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
  if (runServer) {
    Listen (QHostAddress (serverAddrString), serverPort);
  }
  return false;
}

bool
HttpServer::Stop ()
{
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
  HttpSender * sender = new HttpSender (sock, this, db);
  connect (sender, SIGNAL (finished()), sender, SLOT (deleteLater ()));
  sender->start ();
}


} // namespace

