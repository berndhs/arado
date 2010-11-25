#ifndef CONNECTION_DISPLAY_H
#define CONNECTION_DISPLAY_H
///
/// Connection-Display
///

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

#include "ui_connection-display.h"
#include "arado-peer.h"

class QTimer;

namespace arado
{

class DBManager;

class ConnectionDisplay : public QWidget 
{
Q_OBJECT

public:


  enum ConnModelData {
       Conn_Celltype = Qt::UserRole + 2,
       Conn_Level = Qt::UserRole + 3,
       Conn_Uuid  = Qt::UserRole + 4
       };

  enum ConnCellType {
     Cell_None = 0,
     Cell_Nick = 1,
     Cell_Addr = 2,
     Cell_Port = 3
  };
  
  ConnectionDisplay (QWidget *parent=0);

  void Start ();
  void SetDB (DBManager *dbm) { db = dbm; }
  

public slots:


  void ShowPeers ();

  void ChangePeer (AradoPeer & peer);

private slots:

  void DoStartSync ();
  void DoAddDevice ();
  void DoDeleteDevice ();
  void MoveLeft ();
  void MoveRight ();
  void EditListener ();
  void ChangeTrafficParams ();
  void checkAdvViewBox(bool show);

signals:

  void StartSync (bool reload=false);
  void AddDevice ();
  void HaveNewPeer ();
  void WantEditListener ();
  void TrafficParamsChanged ();

private:

  void LoadTrafficParams ();
  void ShowPeers (QTableWidget * table, QString level, AradoPeerList & peers);
  QTableWidgetItem * OnlyOne (QString action = QString(), bool verify= false);
  void Highlight (QTableWidgetItem *item, AradoPeer & peer);
  bool FindPeer  (QString & nick, QTableWidget  ** table, int & row);
  int  FindPeer  (QTableWidget *table, const QString & nick);
  QTableWidgetItem * FindCell (const QTableWidget *table, 
                                 int row, 
                        ConnCellType ct);

  Ui_ConnectionDisplay   ui;

  DBManager             *db;
  bool                   haveNew;
  QTimer                *refreshTimer;

  /**      frequencies are per hour */
  double   urlFreqA;
  double   urlFreqB;
  double   urlFreqC;

  /**      chunks in number of Urls per request */
  int      urlChunkA;
  int      urlChunkB;
  int      urlChunkC;

};

} // arado

#endif
