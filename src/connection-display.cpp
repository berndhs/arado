

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

#include "connection-display.h"
#include "arado-peer.h"
#include "db-manager.h"
#include <QDebug>

namespace arado
{

ConnectionDisplay::ConnectionDisplay (QWidget *parent)
  :QWidget (parent),
   db (0)
{
  ui.setupUi (this);

  connect (ui.buttonStartSync, SIGNAL (clicked()), this, SLOT (DoStartSync()));
  connect (ui.buttonAddDevice, SIGNAL (clicked()), this, SLOT (DoAddDevice()));
}

void
ConnectionDisplay::DoStartSync ()
{
  emit StartSync (haveNew);
  haveNew = false;
}

void
ConnectionDisplay::DoAddDevice ()
{
  emit AddDevice ();
}

void
ConnectionDisplay::AddPeer (QString nick, QString addr, QString addrType,
                       QString level, int port)
{
  qDebug () << " new peer to add " << nick << addr << addrType << level << port;
  AradoPeer newPeer (nick, addr, addrType, level, port);
  bool added (false);
  if (db) {
    added = db->AddPeer (newPeer);
  } else {
    qDebug () << " no DB in connection display ";
  }
  if (added) {
    haveNew = true;
    emit HaveNewPeer ();
  }
}

} // namespace
