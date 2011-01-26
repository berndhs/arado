#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H


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

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QUrl>
#include "db-manager.h"
#include <QMap>
#include "networkaccessmanager.h"

class NetworkAccessManager;
class QNetworkReply;
class QIODevice;
class QBuffer;

namespace arado
{

class Policy;
class AradoStreamParser;
class HttpClientReply;
class AradoPeer;

class HttpAddress {
public:

   HttpAddress (){}

   HttpAddress (const QString & id, const QUrl & u, quint16 p=80)
      :ident (id),
       url(u), 
       port (p),
       useUrl(true) 
       {}
   HttpAddress (const QString & id, const QHostAddress &a, quint16 p=80)
     :ident (id),
      haddr (a),
      port (p),
      useUrl (false)
      {}
   HttpAddress (const HttpAddress & other)
     :ident (other.ident),
      url(other.url),
      haddr (other.haddr),
      port (other.port),
      useUrl (other.useUrl)
      {}
   HttpAddress & operator= (const HttpAddress &other)
      { if (this != &other) {
          ident = other.ident;
          url = other.url;
          haddr = other.haddr;
          port = other.port;
          useUrl = other.useUrl;
        }
        return *this;
      }

   QString        ident;
   QUrl           url;
   QHostAddress   haddr;
   quint16        port;
   bool           useUrl;
};

class HttpClient : public QObject
{
Q_OBJECT

public:

  HttpClient (QObject *parent = 0);

  void SetDB (DBManager * dbm);
  void SetPolicy (Policy * pol);
  
  int AddServer (const QString & id, 
                 const QHostAddress & addr, 
                 quint16 port=80);
  int AddServer (const QString & id,
                 const QUrl & serverUrl, 
                 quint16 port=80);
  void DropServer (int server);
  void DropAllServers ();

  void ReloadServers (const QString & kind = QString ("0"));

public slots:

  void PollPeers (const QStringList & peerList,
                  int   maxRecent = 50,
                  int   maxRandom = 50);
  void PollPeer  (const QString & nick,
                  int   maxRecent = 50,
                  int   maxRandom = 50);


private slots:

  void HandleReply (QNetworkReply * reply);

signals:

  void AddedUrls (int numAdded);
  void AddedPeers (int numAdded);

private:

  typedef QMap <int, HttpAddress> ServerMap;

  void Poll (HttpAddress & addr, const QString & nick, 
             int maxRecent, int maxRandom);
  void PollAddr (HttpAddress & addr, const QString & nick);

  void SendUrlOfferGet (const QUrl & basicUrl, const QString & nick);
  void SendUrlRequestGet (const QUrl & basicUrl,
                          const QString & nick,
                          const QString & kind = QString ("recent"),
                          int   size = 50);
  void SendAddrOfferGet (const QUrl & basicUrl, const QString & nick);
  void SendAddrRequestGet (const QUrl & basicUrl, const QString & nick);

  int ReceiveUrls (AradoStreamParser & parser);
  int ReceiveAddrs (AradoStreamParser & parser);

  void SkipWhite (QIODevice *dev);
  void ProcessRequestReply (HttpClientReply * reply);
  void ProcessOfferReply (HttpClientReply * reply, const QUrl & origUrl);

  void              SaveReply  (HttpClientReply * hcr);
  void              ForgetReply (const HttpClientReply * hcr);
  HttpClientReply * CheckReply (QNetworkReply * nr);


  DBManager   *db;
  Policy      *policy;

  ServerMap              servers;
  QMap <QString, int>    peers;
  int                    nextServer;

  NetworkAccessManager *network;
  bool                   askGet;
  bool                   offerPut;
  bool                   tradeAddr;
  bool                   tradeUrl;
  AradoPeer             *serverSelf;

  QMap   <QNetworkReply*, HttpClientReply*>  replyWait;
  QMap   <HttpClientReply*, QBuffer*>   putWait;



};

} // namespace

#endif
