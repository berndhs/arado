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
#include <QString>
#include <QUrl>
#include <QBuffer>
#include <map>

#ifndef ARADO_HTTP_THREAD
#define ARADO_HTTP_THREAD 0
#endif

class QTcpSocket;

namespace arado
{

class DBManager;
class Policy;

class HttpSender

#if ARADO_HTTP_THREAD
 : public QThread 
#else
 : public QObject
#endif

{
Q_OBJECT

public:

  HttpSender (int sock, QObject *parent, DBManager *dbm, Policy *pol);

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

private:

  void  HandleRequest (const QString & reqType,
                       const QList < QPair <QString,QString > > & items);
  void  ReplyRecent (int maxItems, const QString & datatype );
  void  ReplyRange (bool useNewest, quint64 newest, 
                    bool useOldest, quint64 oldest,
                    const QString & datatype );
  void  ReplyInvalid (const QString & message, int error=400);
  void  ReplyOffer (const QString & datatype );
  void  ProcessPut (const QString & urlText, const QString & proto);
  void  SkipWhite (QIODevice *dev);

  int         socket;
  QTcpSocket  *tcpSocket;
  DBManager   *db;
  Policy      *policy;
  QBuffer      inbuf;
  int          expectSize;
  bool         collectingPut;
  QString      putUrl;
  QString      putProto;

  std::map <QString, QString>  expectType;

};

} // namespace

#endif
