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
#include <map>

class QNetworkAccessManager;
class QNetworkReply;
class QIODevice;

namespace arado
{

class Policy;

class HttpAddress {
public:

   HttpAddress (){}

   HttpAddress (const QUrl & u, quint16 p=80)
      :url(u), 
       port (p),
       useUrl(true) 
       {}
   HttpAddress (const QHostAddress &a, quint16 p=80)
     :haddr (a),
      port (p),
      useUrl (false)
      {}
   HttpAddress (const HttpAddress & other)
     :url(other.url),
      haddr (other.haddr),
      port (other.port),
      useUrl (other.useUrl)
      {}
   HttpAddress & operator= (const HttpAddress &other)
      { if (this != &other) {
          url = other.url;
          haddr = other.haddr;
          port = other.port;
          useUrl = other.useUrl;
        }
        return *this;
      }

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
  
  int AddServer (const QHostAddress & addr, quint16 port=80);
  int AddServer (const QUrl & serverUrl, quint16 port=80);
  void DropServer (int server);
  void DropAllServers ();
  void PollServer (int server);

public slots:

  void Poll ();

private slots:

  void HandleReply (QNetworkReply * reply);

signals:

  void AddedUrls (int numAdded);

private:

  void Poll (HttpAddress & addr);
  void SkipWhite (QIODevice *dev);
  void ProcessRequestReply (QNetworkReply * reply);
  void ProcessOfferReply (QNetworkReply * reply);

  typedef std::map <int, HttpAddress>  ServerMap;

  DBManager   *db;
  Policy      *policy;

  ServerMap   servers;
  int         nextServer;

  QNetworkAccessManager *network;

  QList  <QNetworkReply*>  requestWait;
  QList  <QNetworkReply*>  offerWait;


};

} // namespace

#endif
