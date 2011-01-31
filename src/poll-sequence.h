#ifndef POLL_SEQUENCE_H
#define POLL_SEQUENCE_H

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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QSet>
#include <QTimer>

#include "arado-peer.h"
#include "http-client.h"

namespace arado
{

class DBManager;

class PollSequence : public QObject
{
Q_OBJECT

public:

  PollSequence (QObject *parent=0);

  void SetDB (DBManager *dbm) { db = dbm; }
  void Start ();
  void Stop ();

  void PollClient (HttpClient * httpClient, bool force = 0);

public slots:

  void RefreshParams ();
  void Restart ();

private slots:

  void  PollA ();
  void  PollB ();
  void  PollC ();
  void  SaveParams ();

private:

  void    ComputePeriods (int countA, int countB, int countC);
  void    FindRatios ();
  void    FindRatio (QString level);
  void    SaveRatios ();
  void    SaveRatio (QString level);
  void    RestartTimers ();
  void    Poll (QSet <QString> & nickSet,
                QSet <QString>::iterator  & nickIt,
                int  numRecent,
                int  numRandom);

  DBManager  *db;
  HttpClient *client;

  /**      frequencies are per hour */
  double   urlFreqA;
  double   urlFreqB;
  double   urlFreqC;

  /**      periods in milliseconds */
  int      periodA;
  int      periodB;
  int      periodC;

  /**      max chunk counts for random/recent */
  QMap <QString, int>  maxRecent;
  QMap <QString, int>  maxRandom;

  QSet <QString>  nicksA;
  QSet <QString>  nicksB;
  QSet <QString>  nicksC;

  QSet <QString>::iterator  pollItA;
  QSet <QString>::iterator  pollItB;
  QSet <QString>::iterator  pollItC;

  QTimer    timerA;
  QTimer    timerB;
  QTimer    timerC;

} ; 

} // namespace

#endif
