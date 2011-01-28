
#include "search.h"

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

#include <QTimer>
#include <QDebug>

namespace arado
{

Search::Search (QObject * parent)
  :QObject (parent),
   db (0),
   searchId (0),
   busy (false)
{
}

void 
Search::SetDB (DBManager * dbm)
{
  db = dbm;
}

int
Search::ExactKeywords  (const QStringList & keyList,
                           Combine combine,
                           Result resType )
{
qDebug () << " they want search for list " << keyList;
  return -1;
}

int 
Search::ExactKeyword (const QString & key,
                           Combine combine ,
                           Result resType )
{
qDebug () << " they want search for single " << key;
  if (busy) {
    return -1;
  }
  keys.clear ();
  keys.append (key);
  results.clear ();
  searchId++;
  QTimer::singleShot (10, this, SLOT (DoSearch()));
  return searchId;
}

int
Search::Or (const QString & keystring)
{
qDebug () << " Liberal search";
  if (busy) {
    return -1;
  }
  keys.clear ();
  keys.append (keystring.split (QRegExp("\\s+"),QString::SkipEmptyParts));
  results.clear ();
  searchId++;
  QTimer::singleShot (10,this,SLOT (DoAnySearch ()));
  return searchId;
}

int
Search::And (const QString & keystring)
{
qDebug () << " Liberal search";
  if (busy) {
    return -1;
  }
  keys.clear ();
  keys.append (keystring.split (QRegExp("\\s+"),QString::SkipEmptyParts));
  results.clear ();
  searchId++;
  QTimer::singleShot (10,this,SLOT (DoAllSearch ()));
  return searchId;
}

int
Search::Hash (const QString & keystring)
{
qDebug () << " Liberal search";
  if (busy) {
    return -1;
  }
  keys.clear ();
  keys.append (keystring.split (QRegExp("\\s+"),QString::SkipEmptyParts));
  results.clear ();
  searchId++;
  QTimer::singleShot (10,this,SLOT (DoHashSearch ()));
  return searchId;
}
bool
Search::ResultReady (int resultId)
{
  return false;
}

int
Search::ResultSize (int resultId)
{
  return -1;
}

bool
Search::ResultTable (int resultId, QString & tableName)
{
  tableName.clear();
  return false;
}

bool
Search::ResultList (int resultId, QStringList & hashList)
{
  hashList = results;
  return true;
}

bool
Search::Clear (int resultId)
{
  keys.clear ();
  busy = false;
  return true;
}

void
Search::ClearAll ()
{
  keys.clear ();
  busy = false;
}

void
Search::DoSearch ()
{
  if (db) {
    results.clear ();
    db->GetMatching (results, keys);
    emit Ready (searchId);
    busy = false;
  } else {
    results.clear ();
    emit Ready (searchId);
    busy = false;
  }
}

void
Search::DoAnySearch ()
{
  if (db) {
    results.clear ();
    db->SearchAny (results, keys);
    emit Ready (searchId);
    busy = false;
  } else {
    results.clear ();
    emit Ready (searchId);
    busy = false;
  }
}

void
Search::DoAllSearch ()
{
  if (db) {
    results.clear ();
    db->SearchAll (results, keys);
    emit Ready (searchId);
    busy = false;
  } else {
    results.clear ();
    emit Ready (searchId);
    busy = false;
  }
}

void
Search::DoHashSearch ()
{
  if (db) {
    results.clear ();
    db->SearchByHash (results, keys);
    emit Ready (searchId);
    busy = false;
  } else {
    results.clear ();
    emit Ready (searchId);
    busy = false;
  }
}

} // namespace

