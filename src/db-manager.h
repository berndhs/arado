#ifndef DB_MANAGER_H
#define DB_MANAGER_H

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
#include <QThread>
#include <QSqlDatabase>
#include "arado-url.h"
#include "arado-peer.h"
#include "arado-feed.h"
#include "delib-rng.h"

namespace arado
{

class Policy;

class DBManager : public QThread
{
Q_OBJECT

public:

  enum DBType { 
        DB_None = 0,
        DB_Url = 1,
        DB_Address = 2,
        DB_Rss = 3
        };
  
  DBManager (QObject *parent =0);
  ~DBManager ();

  void  Start ();
  void  Stop ();
  bool  Running () { return dbRunning; }

  void  SetPolicy (Policy *pol);
  bool  AddUrl (AradoUrl & url);
  bool  AddUrl (Policy *pol, AradoUrl & url);
  bool  AddKeywords (AradoUrl & url);

  bool  AddPeer (AradoPeer & peer);
  bool  ReadPeerS (const QString & peerid, AradoPeer &peer);
  bool  ReadPeerU (const QUuid   & uuid,   AradoPeer &peer);
  bool  HavePeerS (const QString & peerid);
  bool  HavePeerU (const QUuid & uuid);
  void  RemovePeerS (const QString & peerid);
  void  MovePeer (const QString & peerid, 
                  const QString & newLevel, 
                  const QString & oldLevel);

  void MarkPeerTime (const QString & peerid,
                     qint64 timestamp);
  bool GetPeerTime (const QString & peerid,
                     qint64 & timestamp);

  int   NumPeers (const QString & kind = QString ("0"));

  qint64 NumUrls ();
  qint64 UrlsAdded () { return urlAddCount; }
  qint64 PeersAdded () { return peerAddCount; }

  void ResetUrlAddCount () { urlAddCount = 0; }
  void ResetPeerAddCount () { peerAddCount = 0; }
  
  AradoUrlList  GetRecent (int howmany);
  AradoUrlList  GetRandom (int howmany);

  AradoPeerList GetPeers (const QString & kind = QString ("0"));
  AradoPeerList GetPeerAddresses (const QString & peerid,
                                  const QString & peerclass);

  bool  ReadKeywords (const QString & hash, QStringList & list);
  bool  ReadUrl   (const QString & hash, AradoUrl & url);
  bool  ReadTime  (const QString & hash, quint64 & timestamp);

  bool  GetMatching (QStringList & hashList, 
                     const QStringList & keys,
                     bool combineAnd = true);
  bool  SearchAny (QStringList & hashList,
                   const QStringList & keys);

  bool  SearchByHash (QStringList & hashList,
                   const QStringList & hashCandidates);
  bool  WriteFeed (const QString & nick,
                   const QUrl & url);
  bool  RemoveFeed (const QString & nick);

  AradoFeedList  GetFeeds ();
  bool ClearNewFeedItems ();
  bool DeleteNewFeedItem (QString &hash);
  bool AddNewFeedItem (AradoUrl &url);
  bool GetNewFeedItems (QList<AradoUrl> &urls);

  bool  StartTransaction (DBType  t = DB_Url);
  bool  CloseTransaction (DBType  t = DB_Url);

signals:

  void InitComplete (bool ok);

private:

  bool  PrivateAddUrl (AradoUrl & url);

  void StartDB  (QSqlDatabase & db, 
                 const QString & conName, 
                 const QString & dbFilename);
  void CheckFileExists (const QString & filename);
  void CheckDBComplete (QSqlDatabase & db,
                        const QStringList & elements);
  QString ElementType (QSqlDatabase & db, const QString & name);
  void    MakeElement (QSqlDatabase & db, const QString & element);
  bool    AddUrlTimestamp (const QString & hash, quint64 & ts);

  void    FillPeerUuids (AradoPeerList & list);

  AradoUrl  GetOneRandom (qint64  limit);

  QSqlDatabase     ipBase;
  QSqlDatabase     urlBase;
  QSqlDatabase     feedBase;

  bool             ipInTransaction;
  bool             urlInTransaction;
  bool             dbRunning;
  qint64           urlAddCount;
  qint64           peerAddCount;

  Policy          *policy;

  deliberate::Rng_LCG_GN  ranGen;

friend class Policy;

};

} // namespace

#endif
