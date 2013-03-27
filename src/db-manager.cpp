

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

#include "db-manager.h"
#include "deliberate.h"
#include "arado-url.h"
#include "policy.h"
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QUuid>
#include <QDebug>

#ifdef Q_OS_WIN32
#define LIBSPOTON_OS_WINDOWS 1
#endif

extern "C"
{
#include "LibSpotOn/libspoton.h"
}

using namespace deliberate;

namespace arado
{

DBManager::DBManager (QObject * parent)
  :QThread (parent),
   ipInTransaction (false),
   urlInTransaction (false),
   dbRunning (false),
   urlAddCount (0),
   peerAddCount (0),
   policy (0)
{
}

DBManager::~DBManager ()
{
  if (dbRunning) {
    Stop ();
  }
}

void
DBManager::SetPolicy (Policy * pol)
{
  policy = pol;
}

void
DBManager::Start ()
{
  QString dataDir = QDesktopServices::storageLocation
                    (QDesktopServices::DataLocation);
  QString ipbasename = dataDir + QDir::separator() 
                               + QString ("ipbase.sql");
  QString urlbasename = dataDir + QDir::separator() 
                                + QString ("urlbase.sql");
  QString feedbasename = dataDir + QDir::separator() 
                                + QString ("feedbase.sql");

  ipbasename = Settings().simpleValue ("database/ipbase",ipbasename)
                         .toString();
  Settings().setSimpleValue ("database/ipbase",ipbasename);

  urlbasename = Settings().simpleValue ("database/urlbase",urlbasename)
                          .toString();
  Settings().setSimpleValue ("database/urlbase",urlbasename);

  feedbasename = Settings().simpleValue ("database/feedbase",feedbasename)
                          .toString();
  Settings().setSimpleValue ("database/feedbase",feedbasename);

  qDebug () << " ip database name " << ipbasename;
  qDebug () << " url database name " << urlbasename;
  qDebug () << " feed database name " << feedbasename;

  QThread::start();
  StartDB (ipBase, "ipBaseCon", ipbasename);
  ipInTransaction = false;
  StartDB (urlBase, "urlBaseCon", urlbasename);
  urlInTransaction = false;
  StartDB (feedBase, "feedBaseCon", feedbasename);

  QStringList  urlElements ;
  urlElements << "urltable"
              << "urlhashindex"
              << "urldescindex"
              << "urlurlindex"
              << "keywords"
              << "keyindex"
              << "timestamps"
              << "uniquetimeindex"
              << "timeindex"
              ;
  CheckDBComplete (urlBase, urlElements);

  QStringList ipElements ;
  ipElements << "stablepeers"
             << "stableindex"
             << "transientpeers"
             << "transientindex"
             << "ippeers"
             << "ippeerindex"
             << "peeruuid"
             << "uuindex"
             << "uniqueuu"
             << "peerstamps"
             << "peertimeindex"
             ;

  CheckDBComplete (ipBase, ipElements);

  QStringList feedElements;
  feedElements << "rssfeeds"
               << "newfeeditems"
               << "uniquerssfeeds"
               ;
  QStringList cleanFeeds;
  cleanFeeds << "cleannewfeeds";
  CleanDB (feedBase, cleanFeeds);
  CheckDBComplete (feedBase, feedElements);

  QDateTime now = QDateTime::currentDateTime();
  QDateTime then = QDateTime (QDate (2112,12,21));
  qint64 timeDiff = now.msecsTo (then);
  timeDiff += now.secsTo (then);
  ranGen.Seed (qAbs (timeDiff));
  qDebug () << " DBManager random Seed " << qAbs (timeDiff);
  dbRunning = true;

  ClearNewFeedItems();

  emit InitComplete (dbRunning);
}

void
DBManager::Stop ()
{
  if (dbRunning) {
    dbRunning = false;
    ipBase.close ();
    urlBase.close ();
    QThread::exit ();
  }
}

void
DBManager::StartDB (QSqlDatabase & db,
                    const QString & conName, 
                    const QString & dbFilename)
{
  db = QSqlDatabase::addDatabase ("QSQLITE", conName);
  CheckFileExists (dbFilename);
  db.setDatabaseName (dbFilename);
  db.open ();
}

void
DBManager::CheckFileExists (const QString & filename)
{
  QFileInfo info (filename);
  if (!info.exists ()) {
    QDir  dir (info.absolutePath ());
    dir.mkpath (info.absolutePath ());
    QFile  file (filename);
    file.open (QFile::ReadWrite);
    file.write (QByteArray (""));
    file.close();
  }
}

void
DBManager::CheckDBComplete (QSqlDatabase & db,
                            const QStringList & elements)
{
  QString eltName, kind;
  for (int e=0; e<elements.size(); e++) {
    eltName = elements.at(e);
    kind = ElementType (db, eltName).toUpper();
qDebug () << " element " << eltName << " is kind " << kind;
    if (kind != "TABLE" && kind != "INDEX") {
      MakeElement (db, eltName);
    }
  }
}

void
DBManager::CleanDB (QSqlDatabase & db,
                            const QStringList & elements)
{
  for (int e=0; e<elements.size(); e++) {
    QString eltName = elements.at(e);
    MakeElement (db, eltName);
  }
}

QString
DBManager::ElementType (QSqlDatabase & db, const QString & name)
{
  QSqlQuery query (db);
  QString pat ("select * from main.sqlite_master where name=\"%1\"");
  QString cmd = pat.arg (name);
  bool ok = query.exec (cmd);
  if (ok && query.next()) {
    QString tipo = query.value(0).toString();
    return tipo;
  }
  return QString ("none");
}

void
DBManager::MakeElement (QSqlDatabase & db, const QString & element)
{
  QString filename (QString (":/schemas/%1.sql").arg (element));
  QFile  schemafile (filename);
  schemafile.open (QFile::ReadOnly);
  QByteArray  createcommands = schemafile.readAll ();
  schemafile.close ();
  QString cmd (createcommands);
  QSqlQuery query (db);
  query.prepare (cmd);
  bool ok = query.exec ();
qDebug () << " tried " << ok << " to create element with " 
          << query.executedQuery ();
}

void
DBManager::MarkPeerTime (const QString & peerid,
                         qint64 timestamp)
{
  QString cmd ("insert or replace into peerstamps"
               " (peerid, timestamp) "
               " values (?, ?)");
  QSqlQuery mark (ipBase);
  mark.prepare (cmd);
  mark.bindValue (0, QVariant (peerid));
  mark.bindValue (1, QVariant (timestamp));
  bool ok = mark.exec ();
  qDebug () << " tried " << ok << " to mark peer time " 
          << mark.executedQuery ();
}

bool
DBManager::GetPeerTime (const QString & peerid,
                        qint64 & timestamp)
{
  timestamp = -1;
  QString cmd ("select timestamp from peerstamps "
                " where peerid=\"%1\"");
  QSqlQuery select (ipBase);
  bool ok = select.exec (cmd.arg(peerid));
  if (ok && select.next()) {
    timestamp = select.value(0).toLongLong();
    return true;
  }
  return false;
}

bool
DBManager::AddUrl (Policy *pol, AradoUrl & url)
{
  if (pol) {
    return pol->AddUrl (*this, url);
  } else {
    return false;
  }
}

bool
DBManager::AddUrl (AradoUrl & url)
{
  return AddUrl (policy, url);
}

bool
DBManager::PrivateAddUrl (AradoUrl & url)
{
  if (!url.IsValid ()) {
    return false;         // don't want known bad URLs in the database
  }
  StartTransaction(DB_Url);
  QSqlQuery add (urlBase);
  QString cmd ("insert or replace into urltable"
               " (hashid, url, description) "
               " values (?, ?, ?)");
  QString hash (url.Hash());
  if (hash.length() < 1) {
    url.ComputeHash ();
    hash = QString (url.Hash());
  }
  QString desc (url.Description());
  if (desc.length() < 1) {
    desc = tr("No description for this Arado Flashmark.");
  }
  add.prepare (cmd);
  add.bindValue (0, QVariant (hash));
  add.bindValue (1, QVariant (url.Url().toString()));
  add.bindValue (2, QVariant (desc));
  bool ok = add.exec ();
  quint64 ts;
  ok &= AddUrlTimestamp (url.Hash(), ts);
  if (ok) {
    url.SetTimestamp (ts);
    urlAddCount ++;
  }
  CloseTransaction (DB_Url);
  yieldCurrentThread ();
  AddKeywords (url);

  QString description(url.Keywords().join(" "));

  const char *dbpath = 0;
  libspoton_error_t err = LIBSPOTON_ERROR_NONE;
  libspoton_handle_t libspotonHandle;

  if((err = libspoton_init(dbpath,
			   &libspotonHandle)) == LIBSPOTON_ERROR_NONE)
    err = libspoton_save_url
      (url.Url().toEncoded(QUrl::StripTrailingSlash).constData(),
       url.Url().toEncoded(QUrl::StripTrailingSlash).size(),
       url.Description().toUtf8().constData(),
       url.Description().toUtf8().size(),
       description.toUtf8().constData(),
       description.toUtf8().size(),
       "",
       0,
       &libspotonHandle);

  libspoton_close(&libspotonHandle);
  return ok;
}

void
DBManager::RemovePeerS (const QString & peerid)
{
  QString delPattern ("delete from %1 where peerid = \"%2\"");
  QSqlQuery delqry (ipBase);
  delqry.exec (delPattern.arg("stablepeers").arg(peerid));
  delqry.exec (delPattern.arg("transientpeers").arg(peerid));
  delqry.exec (delPattern.arg("ippeers").arg(peerid));
  delqry.exec (delPattern.arg("peeruuid").arg(peerid));
  peerAddCount--;
}

void
DBManager::MovePeer (const QString & peerid,
                     const QString & newLevel,
                     const QString & oldLevel)
{
  QString delPattern ("delete from %1 where peerid = \"%2\"");
  QSqlQuery query (ipBase);
  if (oldLevel == "A" || oldLevel == "B") {
    query.exec (delPattern.arg("stablepeers").arg(peerid));
  } else {
    query.exec (delPattern.arg("transientpeers").arg(peerid));
  }
  QString table;
  if (newLevel == "A" || newLevel == "B") {
    table = "stablepeers";
  } else {
    table = "transientpeers";
  }
  QString insert = QString("insert or replace into %1 "
                " (peerid, peerclass)"
                " values (?, ?)").arg (table);
  query.prepare (insert);
  query.bindValue (0, QVariant (peerid));
  query.bindValue (1, QVariant (newLevel));
  bool ok = query.exec ();
  qDebug () << " re-insert peer command " << insert;
  qDebug () << "           result " << ok << query.executedQuery();
}

bool
DBManager::AddPeer (AradoPeer & peer)
{
  QSqlQuery  add (ipBase);
  QString level = peer.Level ();
  StartTransaction (DB_Address);
  QString table ("stablepeers");
  if (level != "A" && level != "B") {
    table = "transientpeers";
  }
  QString  cmd = QString("insert or replace into %1 "
                " (peerid, peerclass)"
                " values (?, ?)").arg (table);
  add.prepare (cmd);
  add.bindValue (0, QVariant (peer.Nick()));
  add.bindValue (1, QVariant (level));
  bool ok = add.exec ();
  qDebug () << " AddPeer Nick " << peer.Nick() << " level " << level;
  qDebug () << " AddPeer tried " << ok << " " << add.executedQuery();
  if (ok) {
    cmd = QString ("insert or replace into peeruuid "
                   " (peerid, uuid) "
                   " values (?, ?)");
    add.prepare (cmd);
    add.bindValue (0, QVariant (peer.Nick()));
    add.bindValue (1, QVariant (peer.Uuid()));
    ok = add.exec ();
    qDebug () << " AddPeer tried " << ok << " " << add.executedQuery();
  }
  if (ok) {
    cmd = QString ("insert or replace into ippeers "
                   " ( peerid, proto, address, port ) "
                   " values (?, ? , ?, ? )");
    add.prepare (cmd);
    add.bindValue (0, QVariant (peer.Nick ()));
    add.bindValue (1, QVariant (peer.AddrType ()));
    add.bindValue (2, QVariant (peer.Addr ()));
    add.bindValue (3, QVariant (peer.Port ()));
    ok = add.exec ();
    if (ok) {
      peerAddCount ++;
    }
  qDebug () << " AddPeer tried query " << ok << " " << add.executedQuery();
  qDebug () << " AddPeer url        " << peer.Addr ();
  }
  CloseTransaction (DB_Address);
  return ok;
}

bool
DBManager::HavePeerS (const QString & peerid)
{
  QString cmd ("select peerid from ippeers where peerid = \"%1\"");
  QSqlQuery select (ipBase);
  bool ok = select.exec (cmd.arg (peerid));
  if (ok && select.next()) {
    QString pid = select.value (0).toString();
    if (pid == peerid) {
      return true;
    }
  }
  return false;
}

int
DBManager::NumPeers (const QString & kind)
{
  if (kind == "0") {
    return NumPeers ("A") + NumPeers ("B") + NumPeers ("C");
  } else  {
    QSqlQuery select (ipBase);
    QString cmd ("select count(peerid) from %1 "
                          " where %2");
    bool ok = select.exec ((kind == "A" || kind == "B") ?
                             cmd.arg ("stablepeers")
                                .arg (QString ("peerclass = \"%1\"").arg(kind))
                            : cmd.arg ("transientpeers").arg ("1"));
    if (ok && select.next ()) {
      return select.value(0).toInt();
    }                     
  }
  return 0;
}

bool
DBManager::ReadPeerS (const QString & peerid, AradoPeer & peer)
{
  QString cmd (QString ("select address, port from ippeers "
                        " where peerid = \"%1\"")
                        .arg (peerid));
  QSqlQuery select (ipBase);
  bool ok = select.exec (cmd);
  if (ok && select.next ()) {
    peer.SetNick (peerid);
    peer.SetAddr (select.value(0).toString());
    peer.SetPort (select.value(1).toInt());
    cmd = QString ("select uuid from peeruuid "
                   " where peerid = \"%1\"")
                  .arg (peer.Nick());
    ok = select.exec (cmd);
    if (ok && select.next ()) {
      peer.SetUuid (QUuid (select.value(0).toString()));
      return true;
    }
  }
  return false;
}

bool
DBManager::ReadPeerU (const QUuid & uuid, AradoPeer & peer)
{
  QString cmd (QString ("select peerid from peeruuid "
                   " where uuid = \"%1\"")
                  .arg (uuid));
  QSqlQuery select (ipBase);
  bool ok = select.exec (cmd);
  if (ok && select.next ()) {
    peer.SetUuid (uuid);
    peer.SetNick (select.value(0).toString());
    cmd = QString ("select address, port from ippeers "
                   " where peerid = \"%1\"")
                   .arg (peer.Nick());
    ok = select.exec (cmd);
    if (ok && select.next ()) {
      peer.SetAddr (select.value (0).toString());
      peer.SetPort (select.value (1).toInt());
      return true;
    }
  }
  return false;
}

bool
DBManager::HavePeerU (const QUuid & uuid)
{
  QString cmd = QString ("select uuid from peeruuid "
               " where uuid = \"%1\"").arg(uuid.toString());
  QSqlQuery select (ipBase);
  bool ok = select.exec (cmd);
  if (ok && select.next()) {
    QUuid storedUuid = QUuid (select.value(0).toString());
    qDebug () << " look for " << uuid << " found " << storedUuid;
    if (storedUuid == uuid) {
      return true;
    }
  }
  return false;
}

bool
DBManager::AddKeywords (AradoUrl & url)
{
  QStringList keys = url.Keywords ();
  QString hash = QString (url.Hash());
  bool allok (true);
  for (int k=0; k < keys.size(); k++) {
    QSqlQuery add (urlBase);
    QString cmd ("insert into keywords"
                 " (hashid, keyword) "
                 " values (?, ?)");
    add.prepare (cmd);
    add.bindValue (0, hash);
    add.bindValue (1, keys.at(k));
    bool ok = add.exec ();
    allok &= ok;
    yieldCurrentThread ();
  }
  return allok;
}

bool
DBManager::AddUrlTimestamp (const QString & hash, quint64 & ts)
{
  QDateTime now = QDateTime::currentDateTime();
  quint64 secs = now.toTime_t();
  QSqlQuery  stamp (urlBase);
  QString cmd ("insert into timestamps "
               " (hashid, incoming) "
               " values (?, ?)");
  stamp.prepare (cmd);
  stamp.bindValue (0, hash);
  stamp.bindValue (1, secs);
  bool ok = stamp.exec ();
  ts = secs;
  return ok;
}

bool
DBManager::ReadKeywords (const QString & hash, QStringList & list)
{
  list.clear ();
  QSqlQuery select (urlBase);
  QString cmd = QString ("select keyword from keywords "
                         " where hashid = \"%1\"").arg (hash);
  bool ok = select.exec (cmd);
  QString word;
  while (ok && select.next()) {
    word = select.value(0).toString();
    list.append (word);
  }
  return ok;
}

bool
DBManager::ReadUrl (const QString & hash, AradoUrl & url)
{
  QSqlQuery  select (urlBase);
  QString  cmd = QString ("select url, description from urltable "
                          " where hashid = \"%1\"").arg (hash);
  bool ok = select.exec (cmd);

  if (ok && select.next()) {
    QString textUrl = select.value (0).toString ();
    QString desc = select.value (1).toString ();
    AradoUrl newurl;
    newurl.SetUrl (QUrl(textUrl));
    newurl.SetDescription (desc);
    newurl.SetHash (hash.toUtf8());
    QStringList keywords;
    ReadKeywords (hash, keywords);
    newurl.SetKeywords (keywords);
    quint64 stamp (0);
    ReadTime (hash, stamp);
    newurl.SetTimestamp (stamp);
    url = newurl;
    return true;
  }
  return false;
}

bool
DBManager::ReadTime (const QString & hash, quint64 & stamp)
{
  QSqlQuery select (urlBase);
  QString cmd = QString ("select incoming from timestamps "
                         " where hashid = \"%1\"").arg (hash);
  bool ok = select.exec (cmd);
  if (ok && select.next ()) {
    stamp = select.value(0).toULongLong();
  }
  return ok;
}

AradoUrlList
DBManager::GetRecent (int howmany)
{
  AradoUrlList list;
  QSqlQuery  select (urlBase);
  QString cmd = QString ("select hashid, incoming from timestamps where 1 "
               " order by incoming DESC"
               " limit %1").arg (howmany);
  bool ok = select.exec (cmd);
  QString hash;
  quint64 stamp;
  AradoUrl url;
  while (ok && select.next()) {
    hash = select.value (0).toString();
    stamp = select.value (1).toULongLong();
    bool haveUrl = ReadUrl (hash, url);
    if (haveUrl) {
      url.SetTimestamp (stamp);
      list.append (url);
    }
  }
  return list;
}


AradoUrlList
DBManager::GetRandom (int howmany)
{
  AradoUrlList list;
  qint64 maxrow = NumUrls();
  for (int i=0; i<howmany; i++) {
    list.append (GetOneRandom(maxrow));
  }
  return list;
}

qint64
DBManager::NumUrls ()
{
  QString cmd = QString ("select count(rowid) from urltable");
  QSqlQuery select (urlBase);
  bool ok = select.exec (cmd);
  if (ok && select.next()) {
    return select.value(0).toString().toLongLong();
  } else {
    return 0;
  }
}

AradoUrl
DBManager::GetOneRandom (qint64 limit)
{
  QSqlQuery  select (urlBase);
  qint64 rownum = ranGen.Random (qAbs (limit)-1);
qDebug () << " get random row " << rownum << " out of " << limit;
  QString  cmd = QString ("select url, description, hashid from urltable "
                          " where rowid = \"%1\"").arg (rownum);
  bool ok = select.exec (cmd);
  if (ok && select.next ()) {
    QString urlString = select.value(0).toString();
    QString desc = select.value(1).toString();
    QString hash = select.value(2).toString();
    AradoUrl aurl;
    aurl.SetUrl (QUrl (urlString));
    aurl.SetDescription (desc);
    aurl.SetHash (hash.toUtf8());

    QStringList keywords;
    ReadKeywords (hash, keywords);
    aurl.SetKeywords (keywords);
    quint64 stamp (0);
    ReadTime (hash, stamp);
    aurl.SetTimestamp (stamp);
    return aurl;
  }
  return AradoUrl();
}

bool
DBManager::GetMatching (QStringList & hashList, 
                        const QStringList & keys,
                        bool combineAnd)
{
  hashList.clear ();
  if (keys.size() < 1) {
    return false;
  }
  QString cmd ("select hashid from keywords where ");
  cmd.append (QString ("keyword = \"%1\"").arg (keys.at(0)));
  QString combine (combineAnd ? " AND " : " OR ");
  for (int i=1; i<keys.size(); i++) {
    cmd.append (combine);
    cmd.append (QString (" keyword = \"%1\"").arg (keys.at(i)));
  }
  QSqlQuery select (urlBase);
  bool ok = select.exec (cmd);
  QString hash;
  while (ok && select.next ()) {
    hash = select.value(0).toString();
    hashList.append (hash);
  }
  return (ok);
}

bool
DBManager::ClearNewFeedItems ()
{
    QString cmd ("delete from newfeeditems");
    QSqlQuery del (feedBase);
    return del.exec (cmd);
}
bool
DBManager::DeleteNewFeedItem(QString &hash)
{
  QString cmd ("delete from newfeeditems where hashid = ?");

  QSqlQuery del (feedBase);
  del.prepare(cmd);
  del.addBindValue(hash);
  return del.exec ();
}


bool
DBManager::AddNewFeedItem (AradoUrl &url)
{
  qDebug () << "DBManager::AddNewFeedItem" ;

  QString cmd ("insert into newfeeditems (hashid, timein) values(?,?)");
  QSqlQuery insert(feedBase);
  insert.prepare(cmd);
  QString hash(url.Hash());
  insert.bindValue (0, QVariant (hash));
  insert.bindValue (1, QVariant (QDateTime::currentDateTime().toTime_t()));

  bool ok=insert.exec ();
  return ok;
}

bool
DBManager::GetNewFeedItems (QList<AradoUrl> &urls, int maxItems)
{
  qDebug () << "DBManager::GetNewFeedItems";
  QString cmd ("select hashid, timein from newfeeditems "
               " order by timein DESC LIMIT %1");
  urls.clear();
  QSqlQuery select (feedBase);
  bool ok = select.exec (cmd.arg(maxItems));
  while (ok && select.next ()) {
    AradoUrl url;
    QString hash = select.value(0).toString();
    if(ReadUrl(hash,url)) {
        urls.append(url);
    } else {
        qDebug () << "DBManager::GetNewFeedItems, cannot find url hash: "<<hash;
    }
  }
  return ok;
}


bool
DBManager::SearchAny (QStringList & hashList, 
                        const QStringList & keys)
{
  return SearchCompound (hashList, keys, "UNION");
}

bool
DBManager::SearchAll (QStringList & hashList, 
                        const QStringList & keys)
{
  return SearchCompound (hashList, keys, "INTERSECT");
}

bool
DBManager::SearchCompound (QStringList & hashList, 
                        const QStringList & keys,
                        const QString & combine)
{
  hashList.clear ();
  if (keys.size() < 1) {
    return false;
  }
  QString cmd;
  QString coreSelect (" select hashid from "
                        "(select hashid from urltable where "
                        " (description LIKE \"\%%1\%\" "
                        " OR url LIKE \"\%%1\%\" )"
                        " UNION "
                        " select hashid from keywords where "
                        "   keyword LIKE \"\%%1\%\" ) "
                     );
  
  cmd.append (coreSelect.arg (keys.at(0)));
  for (int i=1; i<keys.size(); i++) {
    cmd.append (combine);
    cmd.append (coreSelect.arg (keys.at(i)));
  }
  QSqlQuery select (urlBase);
  bool ok = select.exec (cmd);
  qDebug () << "SEARCH command " << cmd;
  qDebug () << "SEARCH words " << ok << " for " << select.executedQuery();
  QString hash;
  while (ok && select.next ()) {
    hash = select.value(0).toString();
    hashList.append (hash);
  }
  qDebug () << " SEARCH got " << hashList.size() << " results";
  return (ok);
}



bool
DBManager::SearchByHash (QStringList & hashList, 
                        const QStringList & hashCandidates)
{
  hashList.clear ();
  if (hashCandidates.size() < 1) {
    return false;
  }
  QString cmd ("select distinct hashid "
                "from urltable where ");
  QString wherePhrase (" hashid LIKE \"\%%1\%\" ");
  QString combine (" OR ");
  
  cmd.append (wherePhrase.arg (hashCandidates.at(0)));
  for (int i=1; i<hashCandidates.size(); i++) {
    cmd.append (combine);
    cmd.append (wherePhrase.arg (hashCandidates.at(i)));
  }
  QSqlQuery select (urlBase);
  bool ok = select.exec (cmd);
  qDebug () << "SEARCH hashes " << ok << " for " << select.executedQuery();
  QString hash;
  while (ok && select.next ()) {
    hash = select.value(0).toString();
    hashList.append (hash);
  }
  qDebug () << " SEARCH got " << hashList.size() << " results";
  return (ok);
}

AradoPeerList
DBManager::GetPeers (const QString & kind)
{
  AradoPeerList list;
  QString where;
  QString table;
  if (kind == "A") {
    where = QString ("peerclass = \"A\"");
    table = "stablepeers";
  } else if (kind == "B") {
    where = QString ("peerclass = \"B\"");
    table = "stablepeers";
  } else if (kind == "C") {
    where = QString ("1");
    table = "transientpeers";
  } else if (kind == "0") {
    where = QString ("1");
    table = "stablepeers";
  } else {
    return list;
  }
  QString cmd (QString ("select peerid, peerclass from %1 where %2")
               .arg (table)
               .arg (where));
  QSqlQuery  select (ipBase);
  bool ok = select.exec (cmd);
  QString peerid;
  QString peerclass;
  while (ok && select.next ()) {
    peerid = select.value(0).toString();
    peerclass = select.value(1).toString();
    list << GetPeerAddresses (peerid, peerclass);
  }
  qDebug () << " returning " << list.size() << " peers ";
  FillPeerUuids (list);
  return list;
}

AradoPeerList
DBManager::GetPeerAddresses (const QString & peerid, const QString & peerclass)
{
  AradoPeerList  list;
  QString cmd (QString ("select proto, address, port "
                        " from ippeers where peerid =\"%1\" ")
               .arg (peerid));
  QSqlQuery  select (ipBase);
  bool ok = select.exec (cmd);
  QString proto, addr;
  int     port;
  while (ok && select.next ()) {
    proto = select.value (0).toString();
    addr  = select.value (1).toString();
    port  = select.value (2).toInt ();
    list << AradoPeer (peerid, addr, proto, peerclass, port);
    qDebug () << " found " << peerid << addr << proto << peerclass << port;
  }
  return list;
}

void
DBManager::FillPeerUuids (AradoPeerList & list)
{
  AradoPeerList::iterator  it;
  for (it=list.begin(); it != list.end(); it++) {
    QString cmd = QString ("select uuid from peeruuid "
                              " where peerid =\"%1\"")
                 .arg(it->Nick());
    QSqlQuery select (ipBase);
    bool ok = select.exec (cmd);
    if (ok && select.next ()) {
      QString uuidStr = select.value (0).toString();
      it->SetUuid (QUuid(uuidStr));
    }
  }
}

bool
DBManager::WriteFeed (const QString & nick,
                      const QUrl & url)
{
  QSqlQuery  query (feedBase);
  QString   cmd ("insert or replace into rssfeeds (nick, url) "
                  "values (?, ?)");
  query.prepare (cmd);
  query.bindValue (0,QVariant (nick));
  query.bindValue (1,QVariant (url.toString()));
  bool ok = query.exec ();
  qDebug () << " WriteFeed " << ok << query.executedQuery() ;
  return ok;
}

bool
DBManager::RemoveFeed (const QString & nick)
{
  QSqlQuery query (feedBase);
  QString cmd ("delete from rssfeeds where nick='%1'");
  bool ok = query.exec (cmd.arg (nick));
  return ok;
}

AradoFeedList
DBManager::GetFeeds ()
{
  AradoFeedList list;
  QSqlQuery select (feedBase);
  QString cmd ("select nick, url from rssfeeds where 1 "
               " order by nick ASC");
  bool ok = select.exec (cmd);
qDebug () << " feed select query " << ok << select.executedQuery();
  while (ok && select.next ()) {
    QString nick = select.value(0).toString();
    QUrl url = QUrl (select.value(1).toString());
    list.append (AradoFeed (nick, url));
  }
  return list;
}

bool
DBManager::StartTransaction (DBType t)
{
  QSqlQuery * transact (0);
  bool fakeFlag (false);
  bool *busyFlag (&fakeFlag);
  if (t == DB_Url) {
    busyFlag = & urlInTransaction;
    transact = new QSqlQuery (urlBase);
  } else if (t == DB_Address) {
    busyFlag = & ipInTransaction;
    transact = new QSqlQuery (ipBase);
  } else {
    return false;
  }
  if (*busyFlag) {
    return false;
  }
  bool ok = transact->exec ("BEGIN TRANSACTION");  
  if (ok) {
    *busyFlag = true;
  }
  qDebug () << " BEGIN transaction " << ok << transact->lastQuery();
  return ok;
}

bool
DBManager::CloseTransaction (DBType t)
{
  QSqlQuery * transact (0);
  bool fakeFlag (false);
  bool *busyFlag (&fakeFlag);
  if (t == DB_Url) {
    busyFlag = & urlInTransaction;
    transact = new QSqlQuery (urlBase);
  } else if (t == DB_Address) {
    busyFlag = & ipInTransaction;
    transact = new QSqlQuery (ipBase);
  } else {
    return false;
  }
  if (! (*busyFlag)) {
    return false;
  }
  bool ok = transact->exec ("COMMIT TRANSACTION");
  qDebug () << " COMMIT transaction " << ok << transact->lastQuery ();
  if (ok) {
    *busyFlag = false;
  }
  return ok;
}

} // namespace

