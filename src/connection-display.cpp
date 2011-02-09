

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
#include "aradogui.h"
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
   urlFreqC (40.0)
{
  ui.setupUi (this);
  maxRecent["A"] = 50;
  maxRecent["B"] = 50;
  maxRecent["C"] = 50;
  maxRandom["A"] = 50;
  maxRandom["B"] = 50;
  maxRandom["C"] = 50;
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
  QString addr = deliberate::Settings().simpleValue ("http/address").toString();
  quint16 port = deliberate::Settings().simpleValue ("http/port").toUInt();
  bool listening = deliberate::Settings().simpleValue ("http/run").toBool ();
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
  urlFreqA = Settings().simpleValue ("traffic/urlFrequencyA",urlFreqA).toDouble();
  urlFreqB = Settings().simpleValue ("traffic/urlFrequencyB",urlFreqB).toDouble();
  urlFreqC = Settings().simpleValue ("traffic/urlFrequencyC",urlFreqC).toDouble();
  Settings().setSimpleValue ("traffic/urlFrequencyA",urlFreqA);
  Settings().setSimpleValue ("traffic/urlFrequencyB",urlFreqB);
  Settings().setSimpleValue ("traffic/urlFrequencyC",urlFreqC);
  ui.freqABox->setValue (urlFreqA);
  ui.freqBBox->setValue (urlFreqB);
  ui.freqCBox->setValue (urlFreqC);
  QStringList levels;
  levels << "A" << "B" << "C";
  for (int i=0; i<levels.count(); i++) {
    QString lev = levels.at(i);
    maxRecent[lev] = Settings().simpleValue(QString("traffic/maxrecent%1")
                                          .arg(lev),
                                           maxRecent[lev]).toInt();
    maxRandom[lev] = Settings().simpleValue(QString("traffic/maxrandom%1")
                                          .arg(lev),
                                           maxRandom[lev]).toInt();
    Settings().setSimpleValue (QString("traffic/maxrecent%1").arg(lev),
                          maxRecent[lev]);
    Settings().setSimpleValue (QString("traffic/maxrandom%1").arg(lev),
                          maxRandom[lev]);
  }
  ui.ratioSliderA->setMaximum (maxRecent["A"] + maxRandom["A"]);
  ui.ratioSliderA->setValue (maxRecent["A"]);
  ui.ratioSliderB->setMaximum (maxRecent["B"] + maxRandom["B"]);
  ui.ratioSliderB->setValue (maxRecent["B"]);
  ui.ratioSliderC->setMaximum (maxRecent["C"] + maxRandom["C"]);
  ui.ratioSliderC->setValue (maxRecent["C"]);
}

void
ConnectionDisplay::ChangeTrafficParams ()
{
  urlFreqA = ui.freqABox->value ();
  urlFreqB = ui.freqBBox->value ();
  urlFreqC = ui.freqCBox->value ();
  maxRecent["A"] = ui.ratioSliderA->value();
  maxRandom["A"] = ui.ratioSliderA->maximum() - maxRecent["A"];
  maxRecent["B"] = ui.ratioSliderB->value();
  maxRandom["B"] = ui.ratioSliderB->maximum() - maxRecent["B"];
  maxRecent["C"] = ui.ratioSliderC->value();
  maxRandom["C"] = ui.ratioSliderC->maximum() - maxRecent["C"];
  Settings().setSimpleValue ("traffic/urlFrequencyA",urlFreqA);
  Settings().setSimpleValue ("traffic/urlFrequencyB",urlFreqB);
  Settings().setSimpleValue ("traffic/urlFrequencyC",urlFreqC);
  qDebug () << "ConnectionDisplay::ChangeTrafficParams frequencies "
            << urlFreqA << urlFreqB << urlFreqC;
  QStringList levels;
  levels << "A" << "B" << "C";
  for (int i=0; i<levels.count(); i++) {
    QString lev = levels.at(i);
    Settings().setSimpleValue (QString("traffic/maxrecent%1").arg(lev),
                          maxRecent[lev]);
    Settings().setSimpleValue (QString("traffic/maxrandom%1").arg(lev),
                          maxRandom[lev]);
    qDebug () << "ConnectionDisplay::ChangeTrafficParams " << lev 
              << maxRecent[lev] << "/" << maxRandom[lev];
  }
  Settings().sync();
  emit TrafficParamsChanged ();

}

void ConnectionDisplay::checkAdvViewBox(bool show)
{
  LoadTrafficParams ();
  QString addr = deliberate::Settings().simpleValue ("http/address").toString();
  bool listening = deliberate::Settings().simpleValue ("http/run").toBool ();
  bool showaddr = (show || addr == "localhost" || !listening);

  ui.freqABox->setVisible(show);
  ui.freqBBox->setVisible(show);
  ui.freqCBox->setVisible(show);

  ui.restartPollButtonA->setVisible(show);
  ui.restartPollButtonB->setVisible(show);
  ui.restartPollButtonC->setVisible(show);

  ui.labelipask_A->setVisible(show);
  ui.labelipask_B->setVisible(show);
  ui.labelipask_C->setVisible(show);
  ui.label_A_2->setVisible(show);
  ui.label_B_2->setVisible(show);
  ui.label_C_2->setVisible(show);
  ui.ratioSliderA->setVisible(show);
  ui.ratioSliderB->setVisible(show);
  ui.ratioSliderC->setVisible(show);
  ui.buttonDelete->setVisible(show);

  ui.listenAddr->setVisible(showaddr);
  ui.listenPortBox->setVisible(showaddr);
  ui.buttonExternalIp->setVisible(showaddr);
}


} // namespace
