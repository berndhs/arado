

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
ConnectionDisplay::Start ()
{
  ShowPeers ();
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
    ShowPeers ();
  }
}

void
ConnectionDisplay::ShowPeers ()
{
  if (db) {
    AradoPeerList peers = db->GetPeers ("A");
    ShowPeers (ui.tableWidget_A, peers);
    peers = db->GetPeers ("B");
    ShowPeers (ui.tableWidget_B, peers);
    peers = db->GetPeers ("C");
    ShowPeers (ui.tableWidget_C, peers);
  }
}

void
ConnectionDisplay::ShowPeers (QTableWidget * table, AradoPeerList & peers)
{
  if (table == 0) {
    return;
  }
  table->clearContents ();
  table->setRowCount (peers.size());
  table->setEditTriggers (0);
  bool normalSort = table->isSortingEnabled ();
  for (int p=0; p<peers.size(); p++) {
    AradoPeer peer = peers[p];
    table->setSortingEnabled (false);
    QTableWidgetItem * item = new QTableWidgetItem (peer.Nick());
    item->setData (Conn_Celltype, Cell_Nick);
    table->setItem (p, 0, item);
    item = new QTableWidgetItem (peer.Addr ());
    item->setData (Conn_Celltype, Cell_Addr);
    table->setItem (p, 1, item);
    item = new QTableWidgetItem (QString::number (peer.Port()));
    item->setData (Conn_Celltype, Cell_Port);
    table->setItem (p, 2, item);
    table->setSortingEnabled (normalSort);
  }
}

} // namespace
