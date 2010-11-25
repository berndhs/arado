

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
#include "deliberate.h"
#include "aradomain.h"
#include <QItemSelectionModel>
#include <QMessageBox>
#include <QTimer>
#include <QDebug>
#include <QStyle>
#include <QFont>

using namespace deliberate;

namespace arado
{

ConnectionDisplay::ConnectionDisplay (QWidget *parent)
  :QWidget (parent),
   db (0),
   haveNew (true),
   refreshTimer (0),
   urlFreqA (600.0),
   urlFreqB (60.0),
   urlFreqC (40.0),
   urlChunkA (60),
   urlChunkB (40),
   urlChunkC (20)
{
  ui.setupUi (this);

  /* Hide Advanced gui */
  checkAdvViewBox(false);
  connect(ui.checkAdvViewBox, SIGNAL(toggled(bool)), this, SLOT(checkAdvViewBox(bool)));
  //

  connect (ui.buttonStartSync, SIGNAL (clicked()), this, SLOT (DoStartSync()));
  connect (ui.buttonAddDevice, SIGNAL (clicked()), this, SLOT (DoAddDevice()));
  ui.buttonAddDevice->setStyleSheet( "background-color:"
                     " qlineargradient(x1:0, y1:0, x2:0, y2:1, "
                     "stop:0 rgb(255, 255, 224), "
                     "stop:1 rgb(100, 230, 100));" );
  connect (ui.buttonDelete, SIGNAL (clicked()), this, SLOT (DoDeleteDevice()));
  connect (ui.buttonExternalIp, SIGNAL (clicked()),
           this, SLOT (EditListener ()));
  connect (ui.buttonMoveLeft, SIGNAL (clicked()),
           this, SLOT (MoveLeft()));
  connect (ui.buttonMoveRight, SIGNAL (clicked()),
           this, SLOT (MoveRight()));
  connect (ui.restartPollButtonA, SIGNAL (clicked()),
           this, SLOT (ChangeTrafficParams ()));
  connect (ui.restartPollButtonB, SIGNAL (clicked()),
           this, SLOT (ChangeTrafficParams ()));
  connect (ui.restartPollButtonC, SIGNAL (clicked()),
           this, SLOT (ChangeTrafficParams ()));
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
ConnectionDisplay::EditListener ()
{
qDebug () << " connection display want edit listener";
  emit WantEditListener();
}

void
ConnectionDisplay::ShowPeers ()
{
  LoadTrafficParams ();
  QString addr = deliberate::Settings().value ("http/address").toString();
  quint16 port = deliberate::Settings().value ("http/port").toUInt();
  bool listening = deliberate::Settings().value ("http/run").toBool ();
  if (addr == "localhost") {
    ui.listenAddr->setText (tr("<- - - - - Please Configure External Address !"));
  } else {
    ui.listenAddr->setText (addr);
  }
  QFont addrFont = ui.listenAddr->font();
  if (listening) {
    addrFont.setStrikeOut (false);
  } else {
    addrFont.setStrikeOut (true);
  }
  ui.listenAddr->setFont (addrFont);
  QFont portFont = ui.listenPortBox->font();
  if (listening) {
    portFont.setStrikeOut (false);
  } else {
    portFont.setStrikeOut (true);
  }
  ui.listenPortBox->setFont (portFont);
  ui.listenPortBox->setValue (port);
  if (db) {
    AradoPeerList peers = db->GetPeers ("A");
    ShowPeers (ui.tableWidget_A, "A", peers);
    peers = db->GetPeers ("B");
    ShowPeers (ui.tableWidget_B, "B", peers);
    peers = db->GetPeers ("C");
    ShowPeers (ui.tableWidget_C, "C", peers);
    ui.tableWidget_C->setSortingEnabled(true);
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
  mbox.setIconPixmap(QPixmap(":/images/messagebox_info.png"));
  QString msg;
  bool complain (true);
  bool yes (false);
  if (nsel < 1) {
    msg = tr("Please Select an Item");
  } else if (nsel > 1) {
    msg = tr ("Only 1 Item please.");
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
      db->RemovePeerS (nickItem->text());
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
      QString newLevel = AradoPeer::Promote (oldLevel);
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
      QString newLevel = AradoPeer::Demote (oldLevel);
      if (newLevel != oldLevel) {
        db->MovePeer (nickItem->text(), newLevel, oldLevel);
        QTimer::singleShot (500,this,SLOT (ShowPeers()));
      }
    }
  }
}

void
ConnectionDisplay::LoadTrafficParams ()
{
  urlFreqA = Settings().value ("traffic/urlFrequencyA",urlFreqA).toDouble();
  urlFreqB = Settings().value ("traffic/urlFrequencyB",urlFreqB).toDouble();
  urlFreqC = Settings().value ("traffic/urlFrequencyC",urlFreqC).toDouble();
  urlChunkA = Settings().value ("traffic/urlChunkA",urlChunkA).toInt ();
  urlChunkB = Settings().value ("traffic/urlChunkB",urlChunkB).toInt ();
  urlChunkC = Settings().value ("traffic/urlChunkC",urlChunkC).toInt ();
  ui.freqABox->setValue (urlFreqA);
  ui.freqBBox->setValue (urlFreqB);
  ui.freqCBox->setValue (urlFreqC);
  ui.chunkA->setValue (urlChunkA);
  ui.chunkB->setValue (urlChunkB);
  ui.chunkC->setValue (urlChunkC);
}

void
ConnectionDisplay::ChangeTrafficParams ()
{
  urlFreqA = ui.freqABox->value ();
  urlFreqB = ui.freqBBox->value ();
  urlFreqC = ui.freqCBox->value ();
  urlChunkA = ui.chunkA->value ();
  urlChunkB = ui.chunkB->value ();
  urlChunkC = ui.chunkC->value ();
  Settings().setValue ("traffic/urlFrequencyA",urlFreqA);
  Settings().setValue ("traffic/urlFrequencyB",urlFreqB);
  Settings().setValue ("traffic/urlFrequencyC",urlFreqC);
  Settings().setValue ("traffic/urlChunkA",urlChunkA);
  Settings().setValue ("traffic/urlChunkB",urlChunkB);
  Settings().setValue ("traffic/urlChunkC",urlChunkC);
  Settings().sync();  
  emit TrafficParamsChanged ();

}

void ConnectionDisplay::checkAdvViewBox(bool show)
{
    LoadTrafficParams ();
    QString addr = deliberate::Settings().value ("http/address").toString();

    if (show) {
         ui.freqABox->setVisible(true);
         ui.freqBBox->setVisible(true);
         ui.freqCBox->setVisible(true);
         ui.chunkA->setVisible(true);
         ui.chunkB->setVisible(true);
         ui.chunkC->setVisible(true);
         ui.restartPollButtonA->setVisible(true);
         ui.restartPollButtonB->setVisible(true);
         ui.restartPollButtonC->setVisible(true);
         ui.label_URL_A->setVisible(true);
         ui.label_URL_B->setVisible(true);
         ui.label_URL_C->setVisible(true);
         ui.labelipask_A->setVisible(true);
         ui.labelipask_B->setVisible(true);
         ui.labelipask_C->setVisible(true);
         ui.checkBox_I2P->setVisible(true);
         ui.checkboxdisable_C->setVisible(true);
         ui.ipcachesize->setVisible(true);
         ui.labelipcachesize->setVisible(true);
         ui.buttonDelete->setVisible(true);
         ui.listenAddr->setVisible(true);
         ui.listenPortBox->setVisible(true);
         ui.buttonExternalIp->setVisible(true);

    } else {
         ui.freqABox->setVisible(false);
         ui.freqBBox->setVisible(false);
         ui.freqCBox->setVisible(false);
         ui.chunkA->setVisible(false);
         ui.chunkB->setVisible(false);
         ui.chunkC->setVisible(false);
         ui.restartPollButtonA->setVisible(false);
         ui.restartPollButtonB->setVisible(false);
         ui.restartPollButtonC->setVisible(false);
         ui.label_URL_A->setVisible(false);
         ui.label_URL_B->setVisible(false);
         ui.label_URL_C->setVisible(false);
         ui.labelipask_A->setVisible(false);
         ui.labelipask_B->setVisible(false);
         ui.labelipask_C->setVisible(false);
         ui.checkBox_I2P->setVisible(false);
         ui.checkboxdisable_C->setVisible(false);
         ui.ipcachesize->setVisible(false);
         ui.labelipcachesize->setVisible(false);
         ui.buttonDelete->setVisible(false);
         //
         if (addr == "localhost") {
         ui.listenAddr->setVisible(true);
         ui.listenPortBox->setVisible(true);
         ui.buttonExternalIp->setVisible(true);
         } else {
         ui.listenAddr->setVisible(false);
         ui.listenPortBox->setVisible(false);
         ui.buttonExternalIp->setVisible(false);
         }
         //
    }
}


} // namespace
