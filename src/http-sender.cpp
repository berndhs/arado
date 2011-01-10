
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
#include "policy.h"
#include "arado-stream-parser.h"
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>
#include <QUuid>
#include <QBuffer>
#include <QMessageBox>
#include <QTimer>

namespace arado
{

HttpSender::HttpSender (int sock, QObject *parent, DBManager *dbm, Policy *pol,
                       bool letGet, bool letPut, bool talkAddr, bool talkUrl,
                       const QMap <QString, QString> & expectedFrom,
                       const QMap <QString, QString> & expectedType,
                       const QMap <QString, quint64> & accepted)
  :
#if ARADO_HTTP_THREAD
   QThread (parent),
#else
   QObject (parent),
#endif
   socket (sock),
   db (dbm),
   policy (pol),
   grantGet (letGet),
   allowPut (letPut),
   tradeAddr (talkAddr),
   tradeUrl (talkUrl),
   collectingPut (false),
   expectPeer (expectedFrom),
   expectType (expectedType),
   lastAccepted (accepted)
{ 
  tcpSocket = new QTcpSocket (this);
  tcpSocket->setSocketDescriptor (socket);
  connect (tcpSocket, SIGNAL (readyRead()), this, SLOT (Read()));
}

#if ARADO_HTTP_THREAD
void
HttpSender::run ()
#else
void
HttpSender::start ()
#endif
{
  qDebug () << " ---- Sender Socked START ----- ";
  qDebug () << " Sender Thread running";
  qDebug () << " Sender Thread " << this << " socket " 
            << socket << " peer " << tcpSocket->peerAddress();
  qDebug () << " START remote " << tcpSocket->peerAddress ()
            << " port " << tcpSocket->peerPort ();
  qDebug () << " ---------------- ";
#if ARADO_HTTP_THREAD
  QThread::run ();
#endif
}

void
HttpSender::Read ()
{
  qDebug () << " have socked data " << tcpSocket->bytesAvailable();
  if (collectingPut) {
    ReadMore ();
  } else {
    ReadFirst ();
  }
}

void
HttpSender::ReadMore ()
{
  inbuf.buffer().append (tcpSocket->readAll());
  qDebug () << " for sock " << tcpSocket << " from " << tcpSocket->peerAddress();
  qDebug () << " PUT inbuf has " << inbuf.size ()<< " of " 
            << expectSize << " bytes";
  if (inbuf.size() >= expectSize) {
    Complete ();
  }
}

void
HttpSender::Complete ()
{
  qDebug () << " Sender Socket complete, calling ProcessPut " 
             << collectingPut;
  if (collectingPut) {
    inbuf.open (QBuffer::ReadOnly);
    collectingPut = false;
    ProcessPut (putUrl, putProto);
  }
  tcpSocket->close ();
}

void
HttpSender::ReadFirst ()
{
  qDebug () << " ---- Sender Socket " << socket << " READ ----- ";
  qDebug () << " remote addr " << tcpSocket->peerAddress ()
            << " remote port " << tcpSocket->peerPort ();
  qDebug () << " local addr " << tcpSocket->localAddress ()
            << " local port " << tcpSocket->localPort ();
  qDebug () << " socket state " << tcpSocket->state ();
  qDebug () << " bytes available " << tcpSocket->bytesAvailable ();
  qDebug () << " open mode " << tcpSocket->openMode ();
  QHostAddress us = tcpSocket->localAddress ();
  QHostAddress them = tcpSocket->peerAddress ();
  peerAddress = them;
  if (us == them) {
    tcpSocket->close ();
    return; 
  }
  QByteArray msgLine = tcpSocket->readLine (2048);
  QString  request (msgLine);
  qDebug () << " Sender Thread incoming message " << msgLine;
  QStringList parts = request.split (QRegExp ("\\s+"));
  qDebug () << " message parts " << parts;
  if (parts.size() >= 3) {
    QString partCmd = parts.at(0).toUpper();
    QString partUrl = parts.at(1);
    QString partProto = parts.at(2);
    if (partCmd == QString ("GET")) {
      QUrl reqUrl (partUrl);
      qDebug () << " ++++++++++++ url part as qurl " << reqUrl;
      qDebug () << " request path " << reqUrl.path();
      QString path = reqUrl.path().toLower();
      if (path.startsWith (QString ("/arado"))) {
        QList <QPair<QString,QString> > queryParts = reqUrl.queryItems();
        QList <QPair<QString,QString> >::const_iterator cpit;
        for (cpit = queryParts.constBegin(); 
             cpit != queryParts.constEnd(); cpit++) {
          QString left = cpit->first.toLower();
          if (left == QString ("request")
             || left == QString ("offer")) {
            HandleRequest (left, queryParts);
            break;
          }
        }
      } else {
        ReplyInvalid (QString ("Not found"), 404);
      }
    } else if (partCmd == QString ("PUT")) {
      if (!expectPeer.contains(partUrl)) {
        qDebug () << " was not expecting " << partUrl;
        qDebug () << " outstanding offers " << expectPeer;
        ReplyInvalid (QString ("Not found"), 404);
        tcpSocket->close ();
        return;
      }
      putDatatype = expectType [partUrl];
      emit ReceivingData (partUrl);
      qDebug () << this << " accepting expected " << partUrl;
      qDebug () << " outstanding offers " << expectPeer;
      QString msgString;
      expectSize = 0;
      receivingType = "none";
      do {
        msgLine = tcpSocket->readLine();
        msgString = QString(msgLine).trimmed().toLower();
        qDebug () << " checking  " << msgString;
        if (msgString.startsWith ("content-length:")) {
           expectSize = msgString.remove("content-length:")
                                 .toInt();
        } else if (msgString.startsWith ("x-data-type:")) {
           receivingType = msgString.remove ("x-data-type:")
                                    .trimmed()
                                    .toUpper();
           qDebug () << " set x-date-type to " << receivingType;
        }
        qDebug () << " discarded " << msgLine;
      } while (msgString.length() > 0);
      collectingPut = true;
      putUrl = partUrl;
      putProto = partProto;
      ReadMore ();
    }
  }
  qDebug () << " ---------------- ";
}

void
HttpSender::HandleRequest (const QString & reqType,
                           const  QList <QPair <QString, QString> > & items)
{
  int  maxItems (2);
  quint64  oldest (0);
  bool     useOldest (false);
  quint64  newest (0);
  bool     useNewest (false);
  bool     cmdRecent (false);
  bool     cmdRange (false);
  bool     cmdRandom (false);
  bool     isRequest (false);
  bool     isOffer (false);
  bool     isBad (false);
  bool     wrongType (false);
  QString  datatype ("URL");
  QString  level ("0");
  QList <QPair<QString,QString> >::const_iterator cpit;
  QStringList leftList;
  isOffer = (reqType == QString("offer"));
  isRequest = (reqType == QString ("request"));
  isBad = isOffer && isRequest;
  for (cpit = items.constBegin(); cpit != items.constEnd(); cpit++) {
    QString left = cpit->first.toLower ();
    QString right = cpit->second.toLower ();
    leftList << left;
    if (left == QString ("request")) {
      cmdRecent |= (right == QString ("recent"));
      cmdRange  |= (right == QString ("range"));
      cmdRandom |= (right == QString ("random"));
      isBad = isOffer;
    } else if (left == QString ("count")) {
      maxItems = right.toInt();
    } else if (left == QString ("newest")) {
      useNewest = true;
      newest = right.toInt ();
    } else if (left == QString ("oldest")) {
      useOldest = true;
      oldest = right.toInt ();
    } else if (left == QString ("offer")) {
      isBad = isRequest;
    } else if (left == QString ("type")) {
      datatype = right.toUpper();
      if (datatype == "ADDR" && !tradeAddr) {
qDebug () << __LINE__ << " Wrong Type " << datatype;
        wrongType = true;
      } else if (datatype == "URL" && !tradeUrl) {
qDebug () << __LINE__ << " Wrong Type " << datatype;
        wrongType = true;
      }
    } else if (left == QString ("level")) {
      level = right.toUpper ();
    }
    if (isBad) {
      break;
    }
  }
  qDebug () << " LLLLLLLLL left values " << leftList;
  if (isBad) {
    ReplyInvalid (QString ("Invalid Request"));
  } else {
    if (isRequest && !grantGet) {
      ReplyInvalid (QString ("Service Not Provided"),405);
    } else if (isOffer & !allowPut) {
      ReplyInvalid (QString ("Not Accepted"),405);
    } else if (wrongType) {
      ReplyInvalid (QString ("Unsupported Type"),405);
    } else if (isRequest && cmdRecent && !(cmdRange || cmdRandom)) {
      ReplyRecent (maxItems, datatype, level);
    } else if (isRequest && cmdRandom && !(cmdRecent || cmdRange)) {
      ReplyRandom (maxItems, datatype, level);
    } else if (isRequest && cmdRange && !(cmdRecent || cmdRandom)) {
      ReplyRange (useNewest, newest, useOldest, oldest, datatype, level);
    } else if (isOffer && !(cmdRange || cmdRecent)) {
      ReplyOffer (datatype, level);
    } else {
      ReplyInvalid (QString ("Request Syntax Error"));
    }
  }
}

void
HttpSender::ReplyInvalid (const QString & message, int error)
{
  QTextStream ostream (tcpSocket);
  ostream.setAutoDetectUnicode (true);
  QStringList lines;
  lines << QString ("HTTP/1.0 %1 %2\r\n").arg(error).arg(message);
  lines << "Connection: close\r\n";
  lines << "Server: AradoErr/0.1\r\n";
  lines << "\r\n";
  lines << message << "\r\n";
  lines << QDateTime::currentDateTime().toString () << "\n";
  qDebug () << "Reply INVALID sending error message " << lines;
  ostream << lines.join ("");
  tcpSocket->flush ();
  tcpSocket->close ();
}

void
HttpSender::ReplyAck (const QString & message, int status)
{
  QTextStream ostream (tcpSocket);
  ostream.setAutoDetectUnicode (true);
  QStringList lines;
  lines << QString ("HTTP/1.0 %1 %2\r\n").arg(status).arg(message);
  lines << "Connection: close\r\n";
  lines << "Server: AradoAck/0.1\r\n";
  lines << "\r\n";
  ostream << lines.join ("");
  tcpSocket->flush ();
  tcpSocket->close ();
}

void
HttpSender::ReplyRange (bool useNewest, quint64 newest, 
                        bool useOldest, quint64 oldest,
                        const QString & datatype, 
                       const QString & level )
{
  ReplyInvalid (QString ("Not Implemented"), 501);
}

void
HttpSender::ReplySome (int maxItems, const QString & datatype, 
                        const QString & level,
                        bool random)
{
  QStringList lines;
  lines << "HTTP/1.1 200 OK\r\n";
  lines << "Connection: close\r\n";
  lines << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
  lines << QString ("X-Data-Type: %1\r\n").arg(datatype);
  lines << "Server: AradoReq/0.1\r\n";
  lines << "\r\n";
  qDebug () << " sending GOOD reply " << lines.join ("");
  tcpSocket->write (lines.join("").toUtf8());
  if (datatype == QString ("URL")) {
    AradoUrlList urls;
    if (random) {
      urls = db->GetRandom (maxItems);
    } else {
      urls = db->GetRecent (maxItems);
    }
    AradoStreamParser parse;
    parse.SetOutDevice (tcpSocket);
    parse.Write (urls);
  } else if (datatype == QString ("ADDR")) {
    AradoPeerList peers = db->GetPeers ("A");
    peers += db->GetPeers ("B");
    peers += db->GetPeers ("C");
    AradoStreamParser parse;
    parse.SetOutDevice (tcpSocket);
    parse.Write (peers);
  } else {
    QByteArray empty ("<arado/>\r\n");
    tcpSocket->write (empty);
  }
  tcpSocket->flush ();
  tcpSocket->close ();
}

void
HttpSender::ReplyRandom (int maxItems, const QString & datatype,
                        const QString & level)
{
  ReplySome (maxItems, datatype, level, true);
}

void
HttpSender::ReplyRecent (int maxItems, const QString & datatype,
                        const QString & level)
{
  ReplySome (maxItems, datatype, level, false);
}

void
HttpSender::ReplyOffer (const QString & datatype,
                        const QString & level)
{
  if (0 && lastAccepted.contains (peerAddress.toString())) {
    ReplyInvalid (QString ("Too Many Requests"),403);
    return;
  }
  QString uupath = QUuid::createUuid().toString();
  uupath.chop (1);
  uupath.remove (0,1);
  QString wholePath (QString("/aradouu/%1").arg(uupath));
qDebug () << " offer expecting " << wholePath;
  QStringList lines;
  lines << "HTTP/1.1 200 OK\r\n";
  lines << "Connection: close\r\n";
  lines << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
  lines << QString ("X-Data-Type: %1;\r\n").arg(datatype);
  lines << "Server: AradoOff/0.1\r\n";
  lines << "\r\n";
  tcpSocket->write (lines.join("").toUtf8());
  AradoStreamParser parse;
  QBuffer buf;
  buf.open (QBuffer::WriteOnly);
  parse.SetOutDevice (&buf);
  parse.WriteUuPath (uupath, QString ("send"));
  qDebug () << " UUPath message " << buf.buffer();
  tcpSocket->write (buf.buffer());
  tcpSocket->flush ();
  tcpSocket->close ();
  emit ExpectData (wholePath, peerAddress.toString(), datatype);

  qDebug () << " Accepting Offer type " << datatype << " at " << uupath;
}

void
HttpSender::ProcessPut (const QString & urlText, const QString & proto)
{
  qDebug () << "HttpSender " << this << " received expected PUT " << urlText;
  qDebug () << expectPeer;
  if (inbuf.isReadable()) {
    AradoStreamParser parser;
    QBuffer bigbuf;
    bigbuf.setData (inbuf.readAll());
    qDebug () << "BIG Buffer size " << bigbuf.buffer().size();
    qDebug () << " BIG receiving type " << receivingType;
    qDebug () << "BIG Buffer content " << bigbuf.buffer();
    bigbuf.open (QBuffer::ReadOnly);
    SkipWhite (&bigbuf);
    parser.SetInDevice (&bigbuf, false);
    if (receivingType == "URL") {
      ReceiveUrls (parser);
    } else if (receivingType == "ADDR") {
      ReceiveAddrs (parser);
    }
    inbuf.buffer().clear ();
  }
  ReplyAck ();
}


void
HttpSender::ReceiveUrls (AradoStreamParser & parser)
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
HttpSender::ReceiveAddrs (AradoStreamParser & parser)
{
  AradoPeerList peers = parser.ReadAradoPeerList ();
qDebug () << " HttpSender got " << peers.size() << " Peers in message ";
  int numAdded (0);
  bool added (false);
  AradoPeerList::iterator  cuit;
  if (db) {
    quint64 seq = QDateTime::currentDateTime().toTime_t();
    seq *= 10000;
    for (cuit = peers.begin(); cuit != peers.end(); cuit++, seq++) {     
      qDebug () << " HttpSender got peer " ; cuit->DebugDump ();
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
  emit AddedPeers (numAdded);
}

void
HttpSender::SkipWhite (QIODevice *dev)
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

