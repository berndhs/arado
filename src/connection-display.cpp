

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
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTimer>
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
  connect (ui.pushButton_A, SIGNAL (clicked()), this, SLOT (DoAddDevice()));
  connect (ui.pushButton_B, SIGNAL (clicked()), this, SLOT (DoStartSync()));
  connect (ui.pushButton_C, SIGNAL (clicked()), this, SLOT (DoAddDevice()));
  connect (ui.buttonDelete, SIGNAL (clicked()), this, SLOT (DoDeleteDevice()));
  connect (ui.buttonMoveLeft, SIGNAL (clicked()),
           this, SLOT (MoveLeft()));
  connect (ui.buttonMoveRight, SIGNAL (clicked()),
           this, SLOT (MoveRight()));
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
ConnectionDisplay::ShowPeers ()
{
  if (db) {
    AradoPeerList peers = db->GetPeers ("A");
    ShowPeers (ui.tableWidget_A, "A", peers);
    peers = db->GetPeers ("B");
    ShowPeers (ui.tableWidget_B, "B", peers);
    peers = db->GetPeers ("C");
    ShowPeers (ui.tableWidget_C, "C", peers);
  }
}

void
ConnectionDisplay::ShowPeers (QTableWidget * table, 
                              QString level, 
                              AradoPeerList & peers)
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
    item->setData (Conn_Level, level);
    item->setData (Conn_Uuid, peer.Uuid().toString());
    item->setToolTip (tr ("UUID %1").arg (peer.Uuid().toString()));
    Highlight (item, peer);
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

void
ConnectionDisplay::Highlight (QTableWidgetItem *item, AradoPeer & peer)
{
  if (item) {
    AradoPeer::PeerState state = peer.State();
    QFont font = item->font ();
    font.setItalic (false);
    font.setBold (false);
    font.setStrikeOut (false);
    switch (state) {
      case AradoPeer::State_Sent:
        font.setBold (true);
        break;
      case AradoPeer::State_Scheduled:
        font.setItalic (true);
        break;
      case AradoPeer::State_Dead:
        font.setStrikeOut (true);
        break;
      default:
        break;
    }
    item->setFont (font);
  }
}

int
ConnectionDisplay::FindPeer (QTableWidget *table, const QString &nick)
{
  if (table == 0) {
    return -1;
  }
  int nr = table->rowCount ();
  int nc = table->columnCount ();
  for (int r=0; r<nr; r++) {
    for (int c=0; c<nc; c++) {
      QTableWidgetItem * item = table->item(r,c);
      if (item) {
        if (ConnCellType(item->data(Conn_Celltype).toInt()) == Cell_Nick
           && item->text () == nick) {
           return r;
        }
      }
    }
  }
  return -1;
}

bool
ConnectionDisplay::FindPeer (QString & nick, QTableWidget **table, int & row)
{
  QTableWidget * t = ui.tableWidget_A;
  int r = FindPeer (t, nick);
  if (r < 0) {
    t = ui.tableWidget_B;
    r = FindPeer (t, nick);
    if (r < 0) {
      t = ui.tableWidget_C;
      r = FindPeer (t, nick);
    }
  }
  row = r;
  (*table) = t;
  return r >= 0;
}

QTableWidgetItem *
ConnectionDisplay::FindCell (const QTableWidget *table,
                             int   row,
                         ConnCellType ct)
{
  if (table == 0) {
    return 0;
  }
  for (int c=0; c<table->columnCount(); c++) {
    QTableWidgetItem *cell = table->item (row, c);
    if (cell) {
      if (ConnCellType (cell->data (Conn_Celltype).toInt()) == ct) {
        return cell;
      }
    }
  }
  return 0;
}

void
ConnectionDisplay::ChangePeer (AradoPeer & peer)
{
  QString nick = peer.Nick ();
  QTableWidget **table (0);
  int          row (-1);
  bool found = FindPeer (nick, table, row);
  if (found) {
    QTableWidgetItem * item = FindCell (*table, row, Cell_Nick);
    if (item) {
      Highlight (item, peer);   
    }
  }
}

QTableWidgetItem *
ConnectionDisplay::OnlyOne (QString action, bool verify)
{
  QList <QTableWidgetItem*> selected = ui.tableWidget_A->selectedItems ();
  selected += ui.tableWidget_B->selectedItems ();
  selected += ui.tableWidget_C->selectedItems ();
  int nsel = selected.size();
  QMessageBox mbox;
  mbox.setWindowTitle(tr("Arado"));
  QString msg;
  bool complain (true);
  bool yes (false);
  if (nsel < 1) {
    msg = tr("Please Select an Item");
  } else if (nsel > 1) {
    msg = tr ("Only 1 Item Please");
  } else {
    complain = false;
    if (verify) {
      msg = action;
      mbox.setStandardButtons (QMessageBox::Yes 
                            | QMessageBox::No);
    } else {
      yes = true;
    }
  }
  mbox.setText (msg);
  if (verify || complain) {
    int choice = mbox.exec ();
    yes = (choice & QMessageBox::Yes);
  }
  if (yes) {
    return selected.at(0);
  } 
  return 0;
}

void
ConnectionDisplay::DoDeleteDevice ()
{
  QTableWidgetItem *item = OnlyOne ("Really Delete?", true);
  if (item && db) {
    QTableWidget * table = item->tableWidget();
    QTableWidgetItem * nickItem = FindCell (table, item->row(), Cell_Nick);
    if (nickItem) {
      db->RemovePeer (nickItem->text());
      QTimer::singleShot (500,this,SLOT (ShowPeers()));
    }
  }
}

void
ConnectionDisplay::MoveLeft ()
{
  QTableWidgetItem * item = OnlyOne ();
  if (item && db) {
    QTableWidget * table = item->tableWidget();
    QTableWidgetItem * nickItem = FindCell (table, item->row(), Cell_Nick);
    if (nickItem) {
      QString oldLevel = nickItem->data (Conn_Level).toString();
      QString newLevel (oldLevel);
      if (oldLevel == "B") {
        newLevel = "A";
      } else if (oldLevel == "C") {
        newLevel = "B";
      }
      if (newLevel != oldLevel) {
        db->MovePeer (nickItem->text(), newLevel, oldLevel);
        QTimer::singleShot (500,this,SLOT (ShowPeers()));
      }
    }
  }
}

void
ConnectionDisplay::MoveRight ()
{
  QTableWidgetItem * item = OnlyOne ();
  if (item && db) {
    QTableWidget * table = item->tableWidget();
    QTableWidgetItem * nickItem = FindCell (table, item->row(), Cell_Nick);
    if (nickItem) {
      QString oldLevel = nickItem->data (Conn_Level).toString();
      QString newLevel (oldLevel);
      if (oldLevel == "B") {
        newLevel = "C";
      } else if (oldLevel == "A") {
        newLevel = "B";
      }
      if (newLevel != oldLevel) {
        db->MovePeer (nickItem->text(), newLevel, oldLevel);
        QTimer::singleShot (500,this,SLOT (ShowPeers()));
      }
    }
  }
}

} // namespace
