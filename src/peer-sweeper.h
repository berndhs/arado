#ifndef ARADO_PEER_SWEEPER_H
#define ARADO_PEER_SWEEPER_H


/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2011, Arado Team
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

#include <QObject>
#include <QTimer>
#include "db-manager.h"

namespace arado
{

class PeerSweeper : public QObject
{
Q_OBJECT

public:

  PeerSweeper (DBManager * dbMgr, QObject *parent=0);

  void Start (int secs=0);
  void Stop ();

  void SetDB (DBManager * dbMgr);

  void SetPeriod (int secs);
  void SetMaxAge (qint64 secs);

  int     Period ();
  qint64  MaxAge ();

  void SingleShot (int secs);

private slots:

  void DoSweep ();

private:

  DBManager      *dbm;
  QTimer         *sweepTimer;
  int             mainPeriod;
  int             maxAge;
  
};

} // namespace


#endif
