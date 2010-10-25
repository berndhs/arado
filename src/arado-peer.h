#ifndef ARADO_PEER_H
#define ARADO_PEER_H

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

#include <QString>
#include <QList>
#include <QUuid>
#include <QDebug>

namespace arado
{
class AradoPeer 
{
public:

  enum  PeerState {
     State_None = 0,
     State_Sent = 1,
     State_Scheduled = 2,
     State_New = 3,
     State_Alive = 4,
     State_Dead = 5
  };

  AradoPeer (QString theNick = QString(), 
                QString theAddr = QString(), 
                QString theAddrType = QString ("0"), 
                QString theLevel = QString ("C"), 
                    int thePort = 0,
                QUuid   theUuid = QUuid(),
                PeerState theState = State_None);

  QString  Nick () const { return nick; } 
  QString  Addr () const { return addr; }
  QString  AddrType () const { return addrType; }
  QString  Level () const { return level; }
  int      Port () const { return port; }
  QUuid    Uuid () const { return uuid; }
  PeerState  State () const { return state ; }

  void  SetNick (const QString & n) { nick = n; }
  void  SetAddr (const QString & a) { addr = a; }
  void  SetAddrType (const QString & at) { addrType = at; }
  void  SetLevel (const QString & l) { level = l; }
  void  SetPort (int p) { port = p; }
  void  SetUuid (const QUuid & u) { uuid = u; }
  void  SetState (PeerState s) { state = s; }

  void  Demote ();
  void  Promote ();

  static QString Demote (const QString & level);
  static QString Promote (const QString & level);

  bool   IsSelfAddr ();

  void  DebugDump ();

private:
  

  QString        nick;
  QString        addr;
  QString        addrType;
  QString        level;
  int            port;
  QUuid          uuid;
  PeerState      state;
} ;

typedef  QList <AradoPeer>  AradoPeerList;

} // namespace

#endif
