#ifndef ADD_PEER_H
#define ADD_PEER_H


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

#include "ui_address-input.h"
#include <QDialog>
#include <QMap>
#include <QUuid>

class QAbstractButton;

namespace arado
{

class DBManager;

class AddPeerDialog : public QDialog
{
Q_OBJECT

public:

  AddPeerDialog (QWidget *parent = 0);
  
  void SetDB (DBManager  *dbm) { db = dbm; }

  void Run ();

private slots:

  void Ok ();
  void Cancel ();
  void AddrReturn ();

signals:

  void NewPeer (QString nick);


private:
  bool AddPeer (QString nick, 
                QString addr, 
                QString addrType, 
                QString level, 
                    int port,
                QUuid   uuid);

  Ui_AddressDialog   addrUi;

  QMap <QAbstractButton *, QString>  addressType;
  QMap <QAbstractButton *, QString>  levelType;

  DBManager    *db;

} ;

} // namespace

#endif
