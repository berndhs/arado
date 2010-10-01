

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

namespace arado
{

Policy::Policy (QObject *parent)
  :QObject (parent),
   sizeLimit (10000)
{
  sizeLimit = deliberate::Settings().value 
                         ("limits/dailycache",sizeLimit).toInt();
  deliberate::Settings().setValue ("limits/dailycache",sizeLimit);
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
      ok = dbm.AddKeywords (url);
      qDebug () << " Policy Accept updating " << ok << url.Url();
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

} // namespace

