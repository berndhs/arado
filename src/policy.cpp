

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
   sizeLimit (10000)
{
  sizeLimit = deliberate::Settings().value 
                         ("policy/dailycache",sizeLimit).toInt();
  deliberate::Settings().setValue ("policy/dailycache",sizeLimit);
  QString path = QDesktopServices::storageLocation 
                        (QDesktopServices::CacheLocation);
  clock.start ();
  
}

bool
Policy::AddUrl (DBManager &dbm, AradoUrl &url)
{
  QByteArray hashOrig = url.Hash ();
  url.ComputeHash ();
  QByteArray hashNew = url.Hash ();
  if (hashNew == hashOrig) {
    bool ok (false);
    if (!IsKnown (hashNew)) {
      AddHash (hashNew);
      ok = dbm.PrivateAddUrl (url);
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

void
Policy::AddHash (const QByteArray & hash)
{
  if (knownHash.size() >= sizeLimit) {
    knownHash.erase (knownHash.begin());
  }
  knownHash.insert (hash);
}

void
Policy::Load (DBManager * dbm)
{
  int  startms = clock.elapsed ();
  int numEntries(0);
  if (dbm) {
    AradoUrlList  latest;  
    latest = dbm->GetRecent (1000);
    AradoUrlList::const_iterator it;
    for (it = latest.constBegin(); it != latest.constEnd(); it++) {
      AddHash (it->Hash ());
      numEntries++;
    }
  }
  int stopms = clock.elapsed ();
  qDebug () << " finish load " << numEntries 
            << " hash entries from t= " << startms
            << " to t= " << stopms;
}

void
Policy::Flush ()
{
  knownHash.clear ();
}

} // namespace

