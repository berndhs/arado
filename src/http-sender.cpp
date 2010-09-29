
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
#include "arado-stream-parser.h"
#include <QTcpSocket>
#include <QByteArray>
#include <QHostAddress>
#include <QDateTime>
#include <QUuid>

namespace arado
{

HttpSender::HttpSender (int sock, QObject *parent, DBManager *dbm)
  :
#if ARADO_HTTP_THREAD
   QThread (parent),
#else
   QObject (parent),
#endif
   socket (sock),
   db (dbm)
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
  qDebug () << " Sender Thread has good socket";
  qDebug () << " open " << tcpSocket->isOpen () 
            << " readable " << tcpSocket->isReadable ()
            << " error " << tcpSocket->errorString ();
  qDebug () << " remote add " << tcpSocket->peerAddress ()
            << " remote port " << tcpSocket->peerPort ();
  qDebug () << " socket state " << tcpSocket->state ();
  qDebug () << " ---------------- ";
#if ARADO_HTTP_THREAD
  QThread::run ();
#endif
}

void
HttpSender::Read ()
{
  qDebug () << " ---- Sender Socked READ ----- ";
  qDebug () << " open " << tcpSocket->isOpen () 
            << " readable " << tcpSocket->isReadable ()
            << " error " << tcpSocket->errorString ();
  qDebug () << " remote add " << tcpSocket->peerAddress ()
            << " remote port " << tcpSocket->peerPort ();
  qDebug () << " local add " << tcpSocket->localAddress ()
            << " local port " << tcpSocket->localPort ();
  qDebug () << " socket state " << tcpSocket->state ();
  qDebug () << " bytes available " << tcpSocket->bytesAvailable ();
  qDebug () << " open mode " << tcpSocket->openMode ();
  QHostAddress us = tcpSocket->localAddress ();
  QHostAddress them = tcpSocket->peerAddress ();
  if (us == them) {
    tcpSocket->close ();
    return; 
  }
  QByteArray msgBytes = tcpSocket->read (1024);
  QString  request (msgBytes);
  qDebug () << " Sender Thread incoming message " << msgBytes;
  QStringList parts = request.split (QRegExp ("\\s+"));
  qDebug () << " message parts " << parts;
  if (parts.size() >= 3) {
    QString partCmd = parts.at(0).toUpper();
    QString partUrl = parts.at(1);
    QString partProto = parts.at(2);
    if (partCmd == QString ("GET")) {
      QUrl reqUrl (partUrl);
      qDebug () << " url part as qurl " << reqUrl;
      qDebug () << " request path " << reqUrl.path();
      QString path = reqUrl.path().toLower();
      if (path == QString ("arado")) {
        QList <QPair<QString,QString> > queryParts = reqUrl.queryItems();
        QList <QPair<QString,QString> >::const_iterator cpit;
        for (cpit = queryParts.constBegin(); 
             cpit != queryParts.constEnd(); cpit++) {
          if (cpit->first.toLower() == QString ("request")) {
            HandleRequest (queryParts);
          break;
          }
        }
      } else {
        ReplyInvalid (QString ("Not found"), 404);
      }
    } else if (partCmd == QString ("PUT")) {
      ProcessPut (partUrl, partProto);
    }
  }
  qDebug () << " ---------------- ";
#if ARADO_HTTP_THREAD

#else
 // emit finished ();
#endif
}

void
HttpSender::HandleRequest (const  QList <QPair <QString, QString> > & items)
{
  int  maxItems (2);
  quint64  oldest (0);
  bool     useOldest (false);
  quint64  newest (0);
  bool     useNewest (false);
  bool     cmdRecent (false);
  bool     cmdRange (false);
  bool     isRequest (false);
  bool     isOffer (false);
  QString  datatype ("URL");
  QList <QPair<QString,QString> >::const_iterator cpit;
  for (cpit = items.constBegin(); cpit != items.constEnd(); cpit++) {
    QString left = cpit->first.toLower ();
    QString right = cpit->second.toLower ();
    if (left == QString ("request")) {
      cmdRecent |= (right == QString ("recent"));
      cmdRange  |= (right == QString ("range"));
      isRequest = true;
    } else if (left == QString ("count")) {
      maxItems = right.toInt();
    } else if (left == QString ("newest")) {
      useNewest = true;
      newest = right.toInt ();
    } else if (left == QString ("oldest")) {
      useOldest = true;
      oldest = right.toInt ();
    } else if (left == QString ("offer")) {
      isOffer = true;
    } else if (left == QString ("type")) {
      datatype = right.toUpper();
    }
  }
  if (isOffer ^ isRequest) {
    if (isRequest && cmdRecent && !cmdRange) {
      ReplyRecent (maxItems, datatype);
    } else if (isRequest && cmdRange && !cmdRecent) {
      ReplyRange (useNewest, newest, useOldest, oldest, datatype);
    } else if (isOffer && !(cmdRange || cmdRecent)) {
      ReplyOffer (datatype);
    } else {
      ReplyInvalid (QString ("Request Syntax Error"));
    }
  } else {
    ReplyInvalid (QString ("Invalid Request"));
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
  lines << "Server: Arado/0.1\r\n";
  lines << "\r\n";
  lines << message;
  lines << "\r\n";
  lines << message;
  lines << QDateTime::currentDateTime().toString () << "\n";
  qDebug () << " sending error message " << lines;
  ostream << lines.join ("");
  tcpSocket->flush ();
  tcpSocket->close ();
}

void
HttpSender::ReplyRange (bool useNewest, quint64 newest, 
                        bool useOldest, quint64 oldest,
                        const QString & datatype )
{
  ReplyInvalid (QString ("Not Implemented"), 501);
}

void
HttpSender::ReplyRecent (int maxItems, const QString & datatype )
{
  QStringList lines;
  lines << "HTTP/1.1 200 OK\r\n";
  lines << "Connection: close\r\n";
  lines << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
  lines << "Server: Arado/0.1\r\n";
  lines << "\r\n";
  qDebug () << " sending GOOD reply " << lines.join ("");
  tcpSocket->write (lines.join("").toUtf8());
  if (datatype == QString ("URL")) {
    AradoUrlList urls = db->GetRecent (maxItems);
    AradoStreamParser parse;
    parse.SetOutDevice (tcpSocket);
    parse.Write (urls);
  } else if (datatype == QString ("IP")) {
    QByteArray empty ("<arado/>\r\n");
    tcpSocket->write (empty);
  } else {
    QByteArray empty ("<arado/>\r\n");
    tcpSocket->write (empty);
  }
  tcpSocket->flush ();
  tcpSocket->close ();
}

void
HttpSender::ReplyOffer (const QString & datatype)
{
  ReplyInvalid (QString ("Method Not Allowed"),405);
  QString uupath = QUuid::createUuid().toString();
  uupath.chop (1);
  uupath.remove (0,1);
  QString wholePath (QString("/aradouu/%1").arg(uupath));
  expectType [wholePath] = datatype;
  QStringList lines;
  lines << "HTTP/1.1 200 OK\r\n";
  lines << "Connection: close\r\n";
  lines << "Content-Type: text/xml; charset=\"utf-8\"\r\n";
  lines << "Server: Arado/0.1\r\n";
  lines << "\r\n";
  tcpSocket->write (lines.join("").toUtf8());
  AradoStreamParser parse;
  parse.SetOutDevice (tcpSocket);
  parse.WriteUuPath (uupath);
  tcpSocket->flush ();
  tcpSocket->close ();

  qDebug () << " Accepting Offer type " << datatype << " at " << uupath;
}

void
HttpSender::ProcessPut (const QString & urlText, const QString & proto)
{
  qDebug () << " received PUT " << urlText;
  ReplyInvalid (QString ("Not Implemented"), 501);
}


} // namespace
