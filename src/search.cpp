
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

namespace arado
{

Search::Search (QObject * parent)
  :QObject (parent),
   db (0)
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
  return -1;
}

int 
Search::ExactKeyword (const QString & key,
                           Combine combine ,
                           Result resType )
{
  return -1;
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
  hashList.clear ();
  return false;
}

bool
Search::Clear (int resultId)
{
  return true;
}

void
Search::ClearAll ()
{
}


} // namespace

