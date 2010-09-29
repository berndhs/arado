
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
#include "arado-stream-parser.h"
#include "policy.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QIODevice>
#include <QMessageBox>
#include <QTimer>

namespace arado
{

HttpClient::HttpClient (QObject *parent)
  :QObject (parent),
   db(0),
   policy (0),
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

void
HttpClient::SetPolicy (Policy * pol)
{
  policy = pol;
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
    QUrl  requestUrl;
    QUrl  offerUrl;
    if (addr.useUrl) {
      requestUrl = addr.url;
    } else {
      QString pattern (addr.haddr.protocol() == QAbstractSocket::IPv6Protocol ?
                       "http://[%1]" : "http://%1");
      requestUrl.setUrl (pattern.arg (addr.haddr.toString()));
    }
    requestUrl.setPort (addr.port);
    requestUrl.setPath ("/arado");
    offerUrl = requestUrl;
    requestUrl.addQueryItem (QString ("request"),QString ("recent"));
    requestUrl.addQueryItem (QString ("count"),QString::number(20));
    QNetworkRequest  req (requestUrl);
    req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
    req.setRawHeader ("User-Agent", "Arado/0.1");
    qDebug () << " network query " << req.url();
    QNetworkReply * reply;
    reply = network->get (req);
    requestWait.append (reply);

    offerUrl.addQueryItem (QString ("offer"),QString ("data"));
    offerUrl.addQueryItem (QString ("type"),QString ("URL"));
    QNetworkRequest offer (offerUrl);
    offer.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
    offer.setRawHeader ("User-Agent", "Arado/0.1");
    qDebug () << " offer query " << offer.url();
    reply = network->get (offer);
    offerWait.append (reply);
  }
}

void
HttpClient::HandleReply (QNetworkReply * reply)
{
  if (requestWait.contains (reply)) {
    requestWait.removeAll (reply);
    ProcessRequestReply (reply);
  } else if (offerWait.contains (reply)) {
    offerWait.removeAll (reply);
    ProcessOfferReply (reply);
  } else {
    reply->deleteLater ();
  }
}

void
HttpClient::ProcessOfferReply (QNetworkReply * reply)
{
  QMessageBox box;
  QByteArray msgBytes = reply->readAll();
  qDebug () << " Offer Reply says " << msgBytes;
  reply->deleteLater();
}


void
HttpClient::ProcessRequestReply (QNetworkReply * reply)
{
  qDebug () << " network reply " << reply;
  QStringList replyMsg;
  replyMsg << QString ("Network Reply");
  replyMsg << QString ("URL %1").arg(reply->url().toString());
  replyMsg << QString ("remote port %1").arg(reply->url().port());
  int err = reply->error ();
  replyMsg << QString ("error %1").arg (err);
  QList <QNetworkReply::RawHeaderPair>  hdrs = reply->rawHeaderPairs();
  for (int i=0; i<hdrs.size (); i++) {
    QString hdrLine (QString ("%1 => %2")
                      .arg (QString (hdrs[i].first))
                      .arg (QString (hdrs[i].second)));
    replyMsg << hdrLine;                    
  }
  if (err == 0) {
    AradoStreamParser parser;
    SkipWhite (reply);
    parser.SetInDevice (reply, false);
    AradoUrlList urls = parser.ReadAradoUrlList ();
qDebug () << " got " << urls.size() << " URLs in message ";=
    int numAdded (0);
    bool added (false);
    AradoUrlList::iterator  cuit;
    if (db) {
      for (cuit = urls.begin(); cuit != urls.end(); cuit++) {
        if (policy) {
          added = policy->AddUrl (*db, *cuit);
        } else {
          added = db->AddUrl (*cuit);
        }
        if (added) {
          numAdded++;
        }
      }
    }
    if (numAdded > 0) {
      emit AddedUrls (numAdded);
    }
  }
  qDebug () << replyMsg;
  reply->deleteLater ();
}

void
HttpClient::SkipWhite (QIODevice *dev)
{
  char w (' ');
  bool ok = dev->getChar (&w);
  while (ok && (w == ' ' || w == '\n' || w == '\t')) {
    ok = dev->getChar (&w);
  }
  if (!(w == ' ' || w == '\n' || w == '\t')) {
    dev->ungetChar (w);
  }

}

void
HttpClient::SkipWhite (QIODevice *dev)
{
  char w (' ');
  bool ok = dev->getChar (&w);
  while (ok && (w == ' ' || w == '\n' || w == '\t')) {
    ok = dev->getChar (&w);
  }
  if (!(w == ' ' || w == '\n' || w == '\t')) {
    dev->ungetChar (w);
  }

}


} // namespace

