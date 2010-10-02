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

namespace arado
{

class DBManager;

class ConnectionDisplay : public QWidget 
{
Q_OBJECT

public:
  
  ConnectionDisplay (QWidget *parent=0);

  void SetDB (DBManager *dbm) { db = dbm; }

private slots:

  void DoStartSync ();
  void DoAddDevice ();

signals:

  void StartSync ();
  void AddDevice ();

private:

  Ui_ConnectionDisplay   ui;

  DBManager             *db;

};

} // arado

#endif
