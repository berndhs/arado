/** PollSequence
  */

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

#include "poll-sequence.h"
#include "db-manager.h"
#include <QDebug>

namespace arado
{

PollSequence::PollSequence (QObject * parent)
  :QObject (parent),
   db (0)
{
}

QStringList
PollSequence::PollBy (quint64 from, quint64 to)
{
  QStringList nickList;
  if (db) {
    AradoPeerList list = db->GetPeers ("A");
    list += db->GetPeers ("B");
    list += db->GetPeers ("C");
    AradoPeerList::const_iterator ait;
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nickList << ait->Nick();
    }
  }
  qDebug () << " poll Limits " << from << " " << to;
  qDebug () << " PollSequence  is " << nickList;
  return nickList;
}

} // namespace

