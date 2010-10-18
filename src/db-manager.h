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

namespace arado
{

class DBManager : public QThread
{
Q_OBJECT

public:

  enum DBType { 
        DB_None = 0,
        DB_Url = 1,
        DB_Address = 2
        };
  
  DBManager (QObject *parent =0);
  ~DBManager ();

  void  Start ();
  void  Stop ();
  bool  Running () { return dbRunning; }

  bool  AddUrl (AradoUrl & url);
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
  
  AradoUrlList  GetRecent (int howmany);

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
  bool  StartTransaction (DBType  t = DB_Url);
  bool  CloseTransaction (DBType  t = DB_Url);

private:

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

  QSqlDatabase     ipBase;
  QSqlDatabase     urlBase;

  bool             ipInTransaction;
  bool             urlInTransaction;
  bool             dbRunning;

};

} // namespace

#endif
