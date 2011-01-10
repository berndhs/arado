#ifndef HTTP_SENDER_H
#define HTTP_SENDER_H

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

#include <QThread>
#include <QList>
#include <QPair>
#include <QMap>
#include <QString>
#include <QUrl>
#include <QBuffer>
#include <QHostAddress>

#ifndef ARADO_HTTP_THREAD
#define ARADO_HTTP_THREAD 0
#endif

class QTcpSocket;

namespace arado
{

class DBManager;
class Policy;
class AradoStreamParser;

class HttpSender

#if ARADO_HTTP_THREAD
 : public QThread 
#else
 : public QObject
#endif

{
Q_OBJECT

public:

  HttpSender (int sock, QObject *parent, DBManager *dbm, Policy *pol,
              bool letGet, bool letPut, bool talkAddr, bool talkUrl,
              const QMap<QString, QString> & expectedFrom,
              const QMap<QString, QString> & expectedType,
              const QMap<QString, quint64> & accepted);

#if ARADO_HTTP_THREAD
  void  run ();
#else
  void  start ();

signals:
  
  void finished ();
#endif

public slots:

  void  Read ();
  void  ReadFirst ();
  void  ReadMore ();
  void  Complete ();

signals:

  void AddedUrls (int numAdded);
  void AddedPeers (int numAdded);
  void ExpectData (QString uupath, QString peer, QString datatype);
  void ReceivingData (QString uupath);

private:

  void  HandleRequest (const QString & reqType,
                       const QList < QPair <QString,QString > > & items);
  void  ReplyRecent (int maxItems, const QString & datatype , 
                     const QString & level);
  void  ReplyRandom (int maxItems, const QString & datatype , 
                     const QString & level);
  void  ReplySome (int maxItems, const QString & datatype , 
                     const QString & level,
                     bool random);
  void  ReplyRange (bool useNewest, quint64 newest, 
                    bool useOldest, quint64 oldest,
                    const QString & datatype, 
                     const QString & level );
  void  ReplyInvalid (const QString & message, int error=400);
  void  ReplyAck (const QString & message = QString("OK"), int status=200);
  void  ReplyOffer (const QString & datatype, 
                     const QString & level );
  void  ProcessPut (const QString & urlText, const QString & proto);
  void  ReceiveUrls (AradoStreamParser & parser);
  void  ReceiveAddrs (AradoStreamParser & parser);
  void  SkipWhite (QIODevice *dev);

  int         socket;
  QTcpSocket  *tcpSocket;
  DBManager   *db;
  Policy      *policy;
  bool         grantGet;
  bool         allowPut;
  bool         tradeAddr;
  bool         tradeUrl;
  QBuffer      inbuf;
  int          expectSize;
  QString      receivingType;
  bool         collectingPut;
  QString      putDatatype;
  QString      putUrl;
  QString      putProto;
  QHostAddress peerAddress;

  const QMap <QString, QString> & expectPeer;
  const QMap <QString, QString> & expectType;
  const QMap <QString, quint64> & lastAccepted;




};

} // namespace

#endif
