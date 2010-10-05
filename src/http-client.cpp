
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
#include <QBuffer>
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
  qDebug () << __LINE__ << " address in map " << servers[nextServer].haddr;
  int ns = nextServer;
  nextServer++;
  return ns;
}

int
HttpClient::AddServer (const QUrl & serverUrl, quint16 port)
{
  servers[nextServer] = HttpAddress (serverUrl, port);
  qDebug () << __LINE__ << " url in map " << servers[nextServer].url;
  int ns = nextServer;
  nextServer++;
  return ns;
}

void
HttpClient::PollServer (int server)
{
  if (servers.contains (server)) {
    Poll (servers[server]);
  }
}

void
HttpClient::DropServer (int server)
{
  servers.remove (server);
}

void
HttpClient::DropAllServers ()
{
  servers.clear ();
}

void
HttpClient::Poll (bool reloadServers)
{
  qDebug () << " HttpClient Poll start reload " << reloadServers;
  if (reloadServers) {
    ReloadServers ();
  }
  ServerMap::iterator sit;
  for (sit = servers.begin(); sit != servers.end(); sit++) {
    Poll (*sit);
  }
  qDebug () << " HttpClient Poll done ";
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
    qDebug () << " CLIENT Polling host " << requestUrl.host();
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
    offerWait[reply] = offer.url();
  }
}

void
HttpClient::HandleReply (QNetworkReply * reply)
{
  if (!finishedCount.contains(reply)) {
    finishedCount[reply] = 0;
  }
  finishedCount[reply] += 1;
  qDebug () << " reply " << reply << " finished " << finishedCount[reply] << " times";
  if (requestWait.contains (reply)) {
    requestWait.removeAll (reply);
    ProcessRequestReply (reply);
  } else if (offerWait.contains (reply)) {
    QUrl  originalUrl = offerWait[reply];
    offerWait.remove (reply);
    ProcessOfferReply (reply, originalUrl);
  } else if (putWait.contains (reply)) {
    QBuffer * putbuf = putWait[reply];
    delete putbuf;
    putWait.remove (reply);
    reply->deleteLater ();
  } else {
    reply->deleteLater ();
  }
}

void
HttpClient::ProcessOfferReply (QNetworkReply * reply, const QUrl & origUrl)
{
  AradoStreamParser parser;
  QBuffer buf;
  buf.setData (reply->readAll());
  buf.open (QBuffer::ReadOnly);
  parser.SetInDevice (&buf);
  SkipWhite (&buf);
  ControlMessage msg = parser.ReadControlMessage ();
  qDebug () << " raw reply " << buf.buffer();
  qDebug () << " Offer Reply command " << msg.Cmd();
  qDebug () << " Offer Reply status " << msg.Value ("status");
  qDebug () << " Offer Reply path " << msg.Value ("uupath");
  reply->deleteLater();
  if (db && network && (msg.Cmd() == QString ("uupath"))
     && (msg.Value ("status") == QString ("send"))) {
    QUrl uploadUrl;
    uploadUrl.setScheme (origUrl.scheme());
    uploadUrl.setHost (origUrl.host());
    uploadUrl.setPort (origUrl.port());
    QString uupath (msg.Value("uupath"));
    if (uupath.length() > 0) {
      uploadUrl.setPath (QString ("/aradouu/") + uupath);
      AradoUrlList urls = db->GetRecent (100);
      QBuffer *data = new QBuffer;
      data->open (QBuffer::WriteOnly);
      parser.SetOutDevice (data);
      parser.Write (urls);
      data->close ();
      qDebug () << " PUT write Buffer size " << data->buffer().size();
      data->open (QBuffer::ReadOnly);
      data->seek (0);
      QNetworkRequest  req (uploadUrl);
      req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
      req.setRawHeader ("User-Agent", "Arado/0.1");
      QNetworkReply *putReply = network->put (req, data);
    }
  }
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
qDebug () << " got " << urls.size() << " URLs in message ";
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
HttpClient::ReloadServers (const QString & kind)
{
  if (db) {
    servers.clear ();
 
    AradoPeerList  peers = db->GetPeers (kind);
    AradoPeerList::const_iterator  it;
    for (it = peers.constBegin(); it != peers.constEnd(); it++) {
      QString tipo = it->AddrType ();
      QString addr = it->Addr ();
      int     port = it->Port ();
      qDebug () << " address " << addr << " type " << tipo << " port " << port;
      if (tipo == "4" || tipo == "6") {
        AddServer (QHostAddress (addr), port);
      } else if (tipo == "name") {
        QUrl url ;
        url.setScheme ("http");
        url.setHost (addr);
        qDebug () << " adding server " << url;
        AddServer (url, port);
      }
    }
  }
}


} // namespace

