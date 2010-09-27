
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
#include "http-client.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QTimer>

namespace arado
{

HttpClient::HttpClient (QObject *parent)
  :QObject (parent),
   db(0),
   nextServer (1),
   network (0)
{
  network = new QNetworkAccessManager (this);
  connect (network, SIGNAL (finished (QNetworkReply *)),
           this, SLOT (HandleReply (QNetworkReply *)));
}

void
HttpClient::SetDB (DBManager * dbm)
{
  db = dbm;
}

int
HttpClient::AddServer (const QHostAddress & addr, quint16 port)
{
qDebug () << " add server " << addr;
  HttpAddress  ha (addr,port);
  servers[nextServer] = ha;
  qDebug () << " address in object " << ha.haddr;
  qDebug () << " address in map " << servers[nextServer].haddr;
  int ns = nextServer;
  nextServer++;
  return ns;
}

int
HttpClient::AddServer (const QUrl & serverUrl, quint16 port)
{
  servers[nextServer] = HttpAddress (serverUrl, port);
  int ns = nextServer;
  nextServer++;
  return ns;
}

void
HttpClient::PollServer (int server)
{
  if (servers.find (server) != servers.end()) {
    Poll (servers[server]);
  }
}

void
HttpClient::DropServer (int server)
{
  servers.erase (server);
}

void
HttpClient::DropAllServers ()
{
  servers.clear ();
}

void
HttpClient::Poll ()
{
  ServerMap::iterator sit;
  for (sit = servers.begin(); sit != servers.end(); sit++) {
    Poll (sit->second);
  }
}

void
HttpClient::Poll (HttpAddress & addr)
{
  if (network) {
    QUrl  url;
    if (addr.useUrl) {
      url = addr.url;
    } else {
      QString pattern (addr.haddr.protocol() == QAbstractSocket::IPv6Protocol ?
                       "http://[%1]" : "http://%1");
      url.setUrl (pattern.arg (addr.haddr.toString()));
    }
    url.setPort (addr.port);
    url.setPath ("/arado");
    url.addQueryItem (QString ("request"),QString ("fresh"));
    url.addQueryItem (QString ("count"),QString::number(1000));
    qDebug () << " network query " << url;
    network->get (QNetworkRequest (url));
  }
}

void
HttpClient::HandleReply (QNetworkReply * reply)
{
  qDebug () << " network reply " << reply;
  QStringList replyMsg;
  replyMsg << QString ("Network Reply");
  replyMsg << QString ("URL %1").arg(reply->url().toString());
  replyMsg << QString ("error %1").arg (reply->error());
  QList <QNetworkReply::RawHeaderPair>  hdrs = reply->rawHeaderPairs();
  for (int i=0; i<hdrs.size (); i++) {
    QString hdrLine (QString ("%1 => %2")
                      .arg (QString (hdrs[i].first))
                      .arg (QString (hdrs[i].second)));
    replyMsg << hdrLine;                    
  }
  qDebug () << replyMsg;
  QMessageBox box;
  box.setText (replyMsg.join ("\n"));
  QTimer::singleShot (15000, &box, SLOT (accept()));
  box.exec ();
}


} // namespace

