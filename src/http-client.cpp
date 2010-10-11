
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
#include "arado-peer.h"
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
   nextServer (111),
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
HttpClient::AddServer (const QString & id, 
                       const QHostAddress & addr, 
                       quint16 port)
{
qDebug () << " add server " << addr;
  HttpAddress  ha (id, addr,port);
  servers[nextServer] = ha;
  peers[id] = nextServer;
  qDebug () << " address in object " << ha.haddr;
  qDebug () << __LINE__ << "entry " << nextServer << " address in map " << servers[nextServer].haddr;
  int ns = nextServer;
  nextServer++;
  return ns;
}

int
HttpClient::AddServer (const QString & id, 
                       const QUrl & serverUrl, 
                       quint16 port)
{
qDebug () << " add server " << serverUrl;
  servers[nextServer] = HttpAddress (id, serverUrl, port);
  peers[id] = nextServer;
  qDebug () << __LINE__ << "entry " << nextServer << " url in map " << servers[nextServer].url;
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
qDebug () << " drop servers " << server;
  servers.remove (server);
}

void
HttpClient::DropAllServers ()
{
qDebug () << " drop ALL servers ";
  servers.clear ();
}

void
HttpClient::PollPeers (const QStringList & peerList)
{
  qDebug () << " CLIENT polling peers " << peerList;
  qDebug () << " nick map " << peers;
  QStringList::const_iterator it;
  for (it = peerList.constBegin(); it != peerList.constEnd(); it++) {
    QString nick = *it;
    if (peers.contains (nick)) {
      int serverNum = peers[nick];
      if (servers.contains (serverNum)) {
        Poll (servers[serverNum]);
        PollAddr (servers[serverNum]);
      } else {
        qDebug () << " NO server " << serverNum << " for " << nick << " server " << servers[serverNum].ident;
      }
    }
  }
}

void
HttpClient::PollAll (bool reloadServers)
{
  qDebug () << " HttpClient Poll start reload " << reloadServers;
  if (reloadServers) {
    ReloadServers ();
  }
  ServerMap::iterator sit;
  for (sit = servers.begin(); sit != servers.end(); sit++) {
    PollAddr (*sit);
    Poll (*sit);
  }
  qDebug () << " HttpClient Poll done ";
}

void
HttpClient::Poll (HttpAddress & addr)
{
  if (network) {
    qDebug () << " POLL try for " << addr.ident;
    QUrl  basicUrl;
    if (addr.useUrl) {
      basicUrl = addr.url;
    } else {
      QString pattern (addr.haddr.protocol() == QAbstractSocket::IPv6Protocol ?
                       "http://[%1]" : "http://%1");
      basicUrl.setUrl (pattern.arg (addr.haddr.toString()));
    }
    qDebug () << " CLIENT Polling host " << basicUrl.host();
    basicUrl.setPort (addr.port);
    basicUrl.setPath ("/arado");

    SendUrlRequestGet (basicUrl);
    SendUrlOfferGet (basicUrl);
  }
}

void
HttpClient::PollAddr (HttpAddress & addr)
{
  if (network) {
    qDebug () << " POLL Addr try for " << addr.ident;
    QUrl  basicUrl;
    if (addr.useUrl) {
      basicUrl = addr.url;
    } else {
      QString pattern (addr.haddr.protocol() == QAbstractSocket::IPv6Protocol ?
                       "http://[%1]" : "http://%1");
      basicUrl.setUrl (pattern.arg (addr.haddr.toString()));
    }
    qDebug () << " CLIENT Polling Addr host " << basicUrl.host();
    basicUrl.setPort (addr.port);
    basicUrl.setPath ("/arado");

    SendAddrRequestGet (basicUrl);
    SendAddrOfferGet (basicUrl);
  }
}


void
HttpClient::SendUrlRequestGet (const QUrl & basicUrl)
{
  QUrl requestUrl (basicUrl);

  requestUrl.addQueryItem (QString ("request"),QString ("recent"));
  requestUrl.addQueryItem (QString ("count"),QString::number(50));
  requestUrl.addQueryItem (QString ("type"),QString ("URL"));
  QNetworkRequest  req (requestUrl);
  req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  req.setRawHeader ("User-Agent", "AradoURG/0.1");
  qDebug () << " Url request query " << req.url();
  QNetworkReply * reply;
  if (network) {
    reply = network->get (req);
  }
  requestUrlWait.append (reply);
}

void
HttpClient::SendUrlOfferGet (const QUrl & basicUrl)
{
  QUrl  offerUrl (basicUrl);
  offerUrl.addQueryItem (QString ("offer"),QString ("data"));
  offerUrl.addQueryItem (QString ("type"),QString ("URL"));
  QNetworkRequest offer (offerUrl);
  offer.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  offer.setRawHeader ("User-Agent", "AradoofferWait");
  qDebug () << " Url offer query " << offer.url();
  QNetworkReply * reply;
  if (network) {
    reply = network->get (offer);
  }
  offerWait[reply] = offer.url();
}

void
HttpClient::SendAddrRequestGet (const QUrl & basicUrl)
{
  QUrl requestUrl (basicUrl);

  requestUrl.addQueryItem (QString ("request"),QString ("recent"));
  requestUrl.addQueryItem (QString ("count"),QString::number(50));
  requestUrl.addQueryItem (QString ("type"),QString ("ADDR"));
  requestUrl.addQueryItem (QString ("level"),QString ("0"));
  QNetworkRequest  req (requestUrl);
  req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  req.setRawHeader ("User-Agent", "AradoARG/0.1");
  qDebug () << " Addr request query " << req.url();
  QNetworkReply * reply;
  if (network) {
    reply = network->get (req);
  }
  requestAddrWait.append (reply);
}

void
HttpClient::SendAddrOfferGet (const QUrl & basicUrl)
{
  QUrl  offerUrl (basicUrl);
  offerUrl.addQueryItem (QString ("offer"),QString ("data"));
  offerUrl.addQueryItem (QString ("type"),QString ("ADDR"));
  offerUrl.addQueryItem (QString ("level"),QString ("0"));
  QNetworkRequest offer (offerUrl);
  offer.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  offer.setRawHeader ("User-Agent", "AradoAOG/0.1");
  qDebug () << " Addr offer query " << offer.url();
  QNetworkReply * reply;
  if (network) {
    reply = network->get (offer);
  }
  offerWait[reply] = offer.url();
}

void
HttpClient::HandleReply (QNetworkReply * reply)
{
  if (!finishedCount.contains(reply)) {
    finishedCount[reply] = 0;
  }
  finishedCount[reply] += 1;
  qDebug () << " reply " << reply << " finished " << finishedCount[reply] << " times";
  qDebug () << " original request url " << reply->request().url();
  qDebug () << " original request headers " << reply->request().rawHeaderList();
  qDebug () << " in url wait " << requestUrlWait.contains (reply);
  qDebug () << " in put wait " << putWait.contains (reply);
  qDebug () << " in offer url wait " << offerWait.contains (reply);
  if (requestUrlWait.contains (reply) || requestAddrWait.contains (reply)) {
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
  qDebug () << " raw reply " << buf.buffer();
  buf.open (QBuffer::ReadOnly);
  buf.seek (0);
  parser.SetInDevice (&buf);
  SkipWhite (&buf);
  ControlMessage msg = parser.ReadControlMessage ();
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
    QString datatype = origUrl.queryItemValue ("type").toUpper();
qDebug () << " Offer Rply datatype " << datatype;
    QString uupath (msg.Value("uupath"));
    if (uupath.length() > 0) {
      uploadUrl.setPath (QString ("/aradouu/") + uupath);
      QBuffer *data = new QBuffer;
      data->open (QBuffer::WriteOnly);
      parser.SetOutDevice (data);
      if (datatype == "URL") {
        AradoUrlList urls = db->GetRecent (100);
        parser.Write (urls);
      } else if (datatype == "ADDR") {
        AradoPeerList peers = db->GetPeers ("A");
        peers += db->GetPeers ("B");
        peers += db->GetPeers ("C");
        parser.Write (peers);
      } 
      data->close ();
      qDebug () << " PUT write Buffer size " << data->buffer().size();
      data->open (QBuffer::ReadOnly);
      data->seek (0);
      QNetworkRequest  req (uploadUrl);
      req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
      req.setRawHeader ("User-Agent", "AradoOR/0.1");
      req.setRawHeader ("X-Data-Type", datatype.toUtf8());
      network->put (req, data);
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
  replyMsg << QString ("req data type ");
  replyMsg << reply->request().url().queryItemValue ("type");
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
    if (requestUrlWait.contains (reply)) {
      requestUrlWait.removeAll (reply);
      ReceiveUrls (parser);
    } else if (requestAddrWait.contains (reply)) {
      requestAddrWait.removeAll (reply);
      ReceiveAddrs (parser);
    }
  }
  qDebug () << replyMsg;
  reply->deleteLater ();
}

void
HttpClient::ReceiveUrls (AradoStreamParser & parser)
{
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

void
HttpClient::ReceiveAddrs (AradoStreamParser & parser)
{
  AradoPeerList peers = parser.ReadAradoPeerList ();
qDebug () << " got " << peers.size() << " Peers in message ";
  int numAdded (0);
  bool added (false);
  AradoPeerList::iterator  cuit;
  if (db) {
    quint64 seq = QDateTime::currentDateTime().toTime_t();
    seq *= 10000;
    for (cuit = peers.begin(); cuit != peers.end(); cuit++, seq++) {
      if (db->HavePeer (cuit->Uuid())) {
        continue;
      }
      cuit->SetNick (tr("AradoElvis%1").arg(seq));
      cuit->Demote ();
      added = db->AddPeer (*cuit);
      if (added) {
        numAdded++;
      }
    }
  }
  if (numAdded > 0) {
    emit AddedPeers (numAdded);
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

void
HttpClient::ReloadServers (const QString & kind)
{
  qDebug () << " HttpClient reload " << kind;
  if (db) {
    AradoPeerList  peers = db->GetPeers (kind);
    AradoPeerList::const_iterator  it;
    for (it = peers.constBegin(); it != peers.constEnd(); it++) {
      QString tipo = it->AddrType ();
      QString addr = it->Addr ();
      int     port = it->Port ();
      qDebug () << " address " << addr << " type " << tipo << " port " << port;
      if (tipo == "4" || tipo == "6") {
        AddServer (it->Nick(), QHostAddress (addr), port);
      } else if (tipo == "name") {
        QUrl url ;
        url.setScheme ("http");
        url.setHost (addr);
        qDebug () << " adding server " << url;
        AddServer (it->Nick(), url, port);
      }
    }
  }
}


} // namespace

