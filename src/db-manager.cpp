

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
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDebug>

using namespace deliberate;

namespace arado
{

DBManager::DBManager (QObject * parent)
  :QThread (parent),
   ipInTransaction (false),
   urlInTransaction (false)
{
}

void
DBManager::Start ()
{
  QThread::start();
  QString dataDir = QDesktopServices::storageLocation
                    (QDesktopServices::DataLocation);
  QString ipbasename = dataDir + QDir::separator() + QString ("ipbase.sql");
  QString urlbasename = dataDir + QDir::separator() + QString ("urlbase.sql");
  ipbasename = Settings().value ("database/ipbase",ipbasename).toString();
  Settings().setValue ("database/ipbase",ipbasename);
  urlbasename = Settings().value ("database/urlbase",urlbasename).toString();
  Settings().setValue ("database/urlbase",urlbasename);
  qDebug () << " ip database name " << ipbasename;
  qDebug () << " url database name " << urlbasename;

  StartDB (ipBase, "ipBaseCon", ipbasename);
  ipInTransaction = false;
  StartDB (urlBase, "urlBaseCon", urlbasename);
  urlInTransaction = false;

  QStringList ipElements;
  CheckDBComplete (ipBase, ipElements);

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
  
}

void
DBManager::Close ()
{
  ipBase.close ();
  urlBase.close ();
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
    if (kind != "TABLE" && kind != "INDEX") {
      MakeElement (db, eltName);
    }
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
  query.exec ();
}

bool
DBManager::AddUrl (AradoUrl & url)
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
    desc = tr("no description");
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
  }
  CloseTransaction (DB_Url);
  AddKeywords (url);
  return ok;
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

