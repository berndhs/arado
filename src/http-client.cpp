
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
#include "http-client-reply.h"
#include "deliberate.h"
#include "networkaccessmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QIODevice>
#include <QMessageBox>
#include <QBuffer>
#include <QTimer>

using namespace deliberate;

namespace arado
{

HttpClient::HttpClient (QObject *parent)
  :QObject (parent),
   db(0),
   policy (0),
   nextServer (111),
   network (0),
   askGet (true),
   offerPut (true),
   tradeAddr (true),
   tradeUrl (true),
   serverSelf (0)
{
  network = new NetworkAccessManager (this);
  connect (network, SIGNAL (finished (QNetworkReply *)),
           this, SLOT (HandleReply (QNetworkReply *)));
  askGet = Settings().simpleValue ("http/getask",askGet).toBool();
  Settings ().setSimpleValue ("http/getask",askGet);
  offerPut = Settings().simpleValue("http/putoffer",offerPut).toBool();
  Settings ().setSimpleValue ("http/putoffer",offerPut);
  tradeAddr = Settings().simpleValue ("transfer/addresses",tradeAddr).toBool();
  Settings().setSimpleValue ("transfer/addresses", tradeAddr);
  tradeUrl = Settings().simpleValue ("transfer/urls",tradeUrl).toBool();
  Settings().setSimpleValue ("transfer/urls", tradeUrl);

  serverSelf = new AradoPeer;
  serverSelf->SetNick ("self");
  serverSelf->SetAddr (Settings().simpleValue ("http/address").toString());
  serverSelf->SetPort (Settings().simpleValue ("http/port").toUInt());
  serverSelf->SetLevel ("A");
  bool selfRunning = Settings().simpleValue("http/run").toBool();
  serverSelf->SetUuid (QUuid(Settings().simpleValue ("personal/uuid").toString()));
  if (selfRunning) {
    serverSelf->SetState (AradoPeer::State_Alive);
  } else {
    serverSelf->SetState (AradoPeer::State_Dead);
  }
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
HttpClient::PollPeers (const QStringList & peerList,
                       int maxRecent, int maxRandom)
{
  qDebug () << " CLIENT polling peers " << peerList;
  qDebug () << " nick map " << peers;
  QStringList::const_iterator it;
  for (it = peerList.constBegin(); it != peerList.constEnd(); it++) {
    PollPeer (*it, maxRecent, maxRandom);
  }
}

void
HttpClient::PollPeer (const QString & nick,
                      int   maxRecent,
                      int   maxRandom)
{
  if (peers.contains (nick)) {
    int serverNum = peers[nick];
    if (servers.contains (serverNum)) {
      if (tradeUrl) {
        Poll (servers[serverNum], nick, maxRecent, maxRandom);
      }
      if (tradeAddr) {
        PollAddr (servers[serverNum], nick);
      }
    } else {
      qDebug () << " NO server " << serverNum << " for " << nick << " server " << servers[serverNum].ident;
    }
  }
}

void
HttpClient::Poll (HttpAddress & addr, const QString & nick, 
                  int maxRecent, int maxRandom)
{
  if (network) {
    qDebug () << " POLL try for " << addr.ident << nick;
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
    basicUrl.setPath ("/arado/query.php");

    if (askGet) {
      SendUrlRequestGet (basicUrl, nick, "recent", maxRecent);
      SendUrlRequestGet (basicUrl, nick, "random", maxRandom);
    }
    if (offerPut) {
      SendUrlOfferGet (basicUrl, nick);
    }
  }
}

void
HttpClient::PollAddr (HttpAddress & addr, const QString & nick)
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
    basicUrl.setPath ("/arado/query.php");

    if (askGet) {
      SendAddrRequestGet (basicUrl, nick);
    }
    if (offerPut) {
      SendAddrOfferGet (basicUrl, nick); 
    }
  }
}


void
HttpClient::SendUrlRequestGet (const QUrl & basicUrl,
                               const QString & nick,
                               const QString & kind,
                               int   size)
{
  QUrl requestUrl (basicUrl);

  requestUrl.addQueryItem (QString ("request"),kind);
  requestUrl.addQueryItem (QString ("count"),QString::number(size));
  requestUrl.addQueryItem (QString ("type"),QString ("URL"));
  QNetworkRequest  req (requestUrl);
  req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  req.setRawHeader ("User-Agent", "AradoURG/0.1");
  qDebug () << " Url request query " << req.url();
  HttpClientReply * reply (0);
  if (network) {
    reply = HttpClientReply::Get (network, req, HRT_Request, HDT_Url, nick);
    SaveReply (reply);
  }
}

void
HttpClient::SendUrlOfferGet (const QUrl & basicUrl, const QString & nick)
{
  QUrl  offerUrl (basicUrl);
  offerUrl.addQueryItem (QString ("offer"),QString ("data"));
  offerUrl.addQueryItem (QString ("type"),QString ("URL"));
  QNetworkRequest offer (offerUrl);
  offer.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  offer.setRawHeader ("User-Agent", "AradoofferWait");
  qDebug () << " Url offer query " << offer.url();
  HttpClientReply * reply;
  if (network) {
    reply = HttpClientReply::Get (network, offer, HRT_Offer, HDT_Url, nick);
    reply->SetOrigUrl (offer.url());
    SaveReply (reply);
  }
}

void
HttpClient::SendAddrRequestGet (const QUrl & basicUrl, const QString & nick)
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
  HttpClientReply * reply;
  if (network) {
    reply = HttpClientReply::Get (network, req, HRT_Request, HDT_Addr, nick);
    SaveReply (reply);
  }
}

void
HttpClient::SendAddrOfferGet (const QUrl & basicUrl, const QString & nick)
{
  QUrl  offerUrl (basicUrl);
  offerUrl.addQueryItem (QString ("offer"),QString ("data"));
  offerUrl.addQueryItem (QString ("type"),QString ("ADDR"));
  offerUrl.addQueryItem (QString ("level"),QString ("0"));
  QNetworkRequest offer (offerUrl);
  offer.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
  offer.setRawHeader ("User-Agent", "AradoAOG/0.1");
  qDebug () << " Addr offer query " << offer.url();
  HttpClientReply * reply;
  if (network) {
    reply = HttpClientReply::Get (network, offer, HRT_Offer, HDT_Addr, nick);
    reply->SetOrigUrl (offer.url());
    SaveReply (reply);
  }
}

void
HttpClient::HandleReply (QNetworkReply * reply)
{
  HttpClientReply  * hcReply = CheckReply (reply);
  qDebug () << " original request url " << reply->request().url();
  qDebug () << " original request headers " << reply->request().rawHeaderList();
  if (hcReply == 0) {
    qDebug () << " Unexpected Reply " << reply;
    return;
  }
  ForgetReply (hcReply);
  HttpRequestType   hrt = hcReply->Type();
  HttpDataType      hdt = hcReply->DataType();
  qDebug () << " reply for Request type " << hrt << " data type " << hdt;
  qDebug () << " status/error " << reply->error();
  QUrl originalUrl;
  switch (hrt) {
  case HRT_Request:
    ProcessRequestReply (hcReply);
    break;
  case HRT_Offer:
    originalUrl = hcReply->OrigUrl ();
qDebug () << " Reply Case 2";
    ProcessOfferReply (hcReply, originalUrl);
    break;
  case HRT_Put:
    delete putWait[hcReply];
    break;
  default:
    break;
  }
  delete hcReply;
}

void
HttpClient::ProcessOfferReply (HttpClientReply * reply, const QUrl & origUrl)
{
  if (reply == 0) {
    return;
  }
  if (reply->Reply() == 0) {
    return;
  }
  AradoStreamParser parser;
  QBuffer buf;
  buf.setData (reply->Reply()->readAll());
  QString peerNick = reply->PeerNick ();
  qDebug () << " raw reply " << buf.buffer();
  buf.open (QBuffer::ReadOnly);
  buf.seek (0);
  parser.SetInDevice (&buf);
  SkipWhite (&buf);
  ControlMessage msg = parser.ReadControlMessage ();
  qDebug () << " Offer Reply command " << msg.Cmd();
  qDebug () << " Offer Reply status " << msg.Value ("status");
  qDebug () << " Offer Reply path " << msg.Value ("uupath");
  if (db && network && (msg.Cmd() == QString ("uupath"))
     && (msg.Value ("status") == QString ("send"))) {
    QUrl uploadUrl;
    uploadUrl.setScheme (origUrl.scheme());
    uploadUrl.setHost (origUrl.host());
    uploadUrl.setPort (origUrl.port());
    QString datatype = origUrl.queryItemValue ("type").toUpper();
    HttpDataType  hdt (HDT_None);
    QString uupath (msg.Value("uupath"));
    if (uupath.length() > 0) {
      uploadUrl.setPath (QString ("/aradouu/") + uupath);
      QBuffer *data = new QBuffer;
      data->open (QBuffer::WriteOnly);
      parser.SetOutDevice (data);
      if (datatype == "URL") {
        AradoUrlList urls = db->GetRecent (50);
        urls += db->GetRandom (50);
        parser.Write (urls);
        hdt = HDT_Url;
      } else if (datatype == "ADDR") {
        AradoPeerList peers ;
        if (serverSelf) {
          if (serverSelf->State() != AradoPeer::State_Dead
              && !serverSelf->Uuid().isNull()) {
            peers << *serverSelf;
            qDebug () << "including self as peer " ; serverSelf->DebugDump ();
          }
        }
        peers += db->GetPeers ("A");
        peers += db->GetPeers ("B");
        peers += db->GetPeers ("C");
        parser.Write (peers);
        hdt = HDT_Addr;
      } 
      data->close ();
      qDebug () << " PUT write Buffer size " << data->buffer().size();
      data->open (QBuffer::ReadOnly);
      data->seek (0);
      QNetworkRequest  req (uploadUrl);
      req.setHeader (QNetworkRequest::ContentTypeHeader, QString ("xml"));
      req.setRawHeader ("User-Agent", "AradoOR/0.1");
      req.setRawHeader ("X-Data-Type", datatype.toUtf8());
qDebug () << " Offer Reply datatype " << datatype;
qDebug () << " Offer Reply upload to " << req.url ();
      HttpClientReply * hr = HttpClientReply::Put (network, req, 
                                      HRT_Put, hdt, peerNick, data);
      putWait[hr] = data;
      SaveReply (hr);
    }
  }
}


void
HttpClient::ProcessRequestReply (HttpClientReply * hcReply)
{
  qDebug () << " network reply " << hcReply;
  QNetworkReply *netReply = hcReply->Reply();
  if (netReply == 0) {
    qDebug () << " Bad Empty reply " << hcReply;
    return;
  }
  QString peerNick = hcReply->PeerNick ();
  QStringList replyMsg;
  replyMsg << QString ("Network Reply");
  replyMsg << QString ("URL %1").arg(netReply->url().toString());
  replyMsg << QString ("remote port %1").arg(netReply->url().port());
  int err = netReply->error ();
  replyMsg << QString ("error %1").arg (err);
  replyMsg << QString ("req data type ");
  replyMsg << netReply->request().url().queryItemValue ("type");
  QList <QNetworkReply::RawHeaderPair>  hdrs = netReply->rawHeaderPairs();
  for (int i=0; i<hdrs.size (); i++) {
    QString hdrLine (QString ("%1 => %2")
                      .arg (QString (hdrs[i].first))
                      .arg (QString (hdrs[i].second)));
    replyMsg << hdrLine;                    
  }
  if (err == 0) {
    AradoStreamParser parser;
#define USE_EXTRA_BUFFER 1
#if USE_EXTRA_BUFFER
    QBuffer inbuf;
    inbuf.setData (netReply->readAll());
    inbuf.open (QBuffer::ReadOnly);
    SkipWhite (&inbuf);
    parser.SetInDevice (&inbuf, false);
qDebug () << " EXTRA BUFFER raw reply " << inbuf.buffer();
#else
    SkipWhite (netReply);
    parser.SetInDevice (netReply, false);
#endif
    if (hcReply->DataType() == HDT_Url) {
      ReceiveUrls (parser);
    } else if (hcReply->DataType() == HDT_Addr) {
      ReceiveAddrs (parser);
    }
    if (db) {
      db->MarkPeerTime (peerNick, QDateTime::currentDateTime().toTime_t());
    }
  }
  qDebug () << replyMsg;
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
qDebug () << " HttpClient got " << peers.size() << " Peers in message ";
  int numAdded (0);
  bool added (false);
  AradoPeerList::iterator  cuit;
  if (db) {
    quint64 seq = QDateTime::currentDateTime().toTime_t();
    seq *= 10000;
    for (cuit = peers.begin(); cuit != peers.end(); cuit++, seq++) {
      qDebug () << " HttpClient got peer " ; cuit->DebugDump ();
      if (db->HavePeerU (cuit->Uuid())) {
        continue;
      }
      cuit->SetNick (tr("AradoElvis_%1").arg(seq));
      cuit->Demote ();
      if (!cuit->IsSelfAddr ()) {
        added = db->AddPeer (*cuit);
      }
      if (added) {
        numAdded++;
      }
    }
  }
  qDebug () << " emit AddedPeers " << numAdded;
  emit AddedPeers (numAdded);
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

void
HttpClient::SaveReply (HttpClientReply * hcr)
{
  if (hcr) {
    replyWait [hcr->Reply()] = hcr;
  }
}

void
HttpClient::ForgetReply (const HttpClientReply * hcr)
{
  replyWait.remove (hcr->Reply());
}

HttpClientReply *
HttpClient::CheckReply (QNetworkReply *nr)
{
  if (nr) {
    if (replyWait.contains (nr)) {
      return replyWait[nr];
    }
  }
  return 0;
}


} // namespace

