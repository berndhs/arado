

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, 
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
  :QObject (parent)
{
}

void
DBManager::Start ()
{
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
  StartDB (urlBase, "urlBaseCon", urlbasename);

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
  qDebug () << " tried to make " << cmd ;
}

bool
DBManager::AddUrl (AradoUrl & url)
{
  QSqlQuery transact (urlBase);
  bool started = transact.exec ("BEGIN TRANSACTION");
  if (!started) {
    return false;
  }
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
  ok &= AddUrlTimestamp (url.Hash());
  ok &= AddKeywords (url);
  transact.exec ("COMMIT TRANSACTION");
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
    QString cmd ("insert or replace into keywords"
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
DBManager::AddUrlTimestamp (const QString & hash)
{
  QDateTime now = QDateTime::currentDateTime();
  quint64 secs = now.toTime_t();
  QSqlQuery  stamp (urlBase);
  QString cmd ("insert or replace into timestamps "
               " (hashid, lastseen) "
               " values (?, ?)");
  stamp.prepare (cmd);
  stamp.bindValue (0, hash);
  stamp.bindValue (1, secs);
  bool ok = stamp.exec ();
  return ok;
}

} // namespace

