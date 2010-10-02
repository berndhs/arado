

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
#include "policy.h"
#include "deliberate.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace arado
{

Policy::Policy (QObject *parent)
  :QObject (parent),
   sizeLimit (10000),
   filename (QString("/tmp/aradohcache.txt"))
{
  sizeLimit = deliberate::Settings().value 
                         ("policy/dailycache",sizeLimit).toInt();
  deliberate::Settings().setValue ("policy/dailycache",sizeLimit);
  QString path = QDesktopServices::storageLocation 
                        (QDesktopServices::CacheLocation);
  filename = path + QDir::separator() + QString ("hcache.dat");
  filename = deliberate::Settings().value 
                         ("policy/hcache",filename).toString();
  deliberate::Settings().setValue ("policy/hcache",filename);
  
}

bool
Policy::AddUrl (DBManager &dbm, AradoUrl &url)
{
  QByteArray hashOrig = url.Hash ();
  url.ComputeHash ();
  QByteArray hashNew = url.Hash ();
  if (hashNew == hashOrig) {
    bool ok (false);
    if (IsKnown (hashNew)) {
#if 0
      ok = dbm.AddKeywords (url);
      qDebug () << " Policy Accept updating " << ok << url.Url();
#endif
    } else {
      AddHash (hashNew);
      ok = dbm.AddUrl (url);
      qDebug () << " Policy Accept saving " << ok << url.Url();
    }
    return ok;
  } else {
    qDebug () << " Policy Reject saving " << url.Url();
    return false;
  }
}

bool
Policy::IsKnown (const QByteArray & hash)
{
  return knownHash.find (hash) != knownHash.end();
}

bool
Policy::AddHash (const QByteArray & hash)
{
  if (knownHash.size() >= sizeLimit) {
    knownHash.erase (knownHash.begin());
  }
  knownHash.insert (hash);
}

void
Policy::Load ()
{
  QFile file (filename);
  bool ok = file.open (QFile::ReadOnly);
  if (ok) {
    while (!file.atEnd()) {
      QByteArray line = file.readLine (2048);
      AddHash (line);
    }
  }
  qDebug () << " after load HCache size " << knownHash.size();
}

void
Policy::Save ()
{
  qDebug () << " save in " << filename;
  QFileInfo info (filename);
  if (!info.exists()) {
     QString cpath = info.absolutePath();
     QDir dir;
     bool worked = dir.mkpath (cpath);
     qDebug () << " mkpath " << worked << " for " << dir;
  }
  QFile file (filename);
  bool canWrite = file.open (QFile::WriteOnly);
  qDebug () << " writing " << knownHash.size() << " entries to HCache " 
            << file.fileName();
  if (canWrite) {
    std::set <QByteArray>::const_iterator  it;
    for (it = knownHash.begin(); it != knownHash.end(); it++) {
       file.write (*it);
       file.putChar ('\n');
    }
    file.close ();
  }
}

} // namespace

