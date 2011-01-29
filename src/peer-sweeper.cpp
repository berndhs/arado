#include "peer-sweeper.h"

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
#include "arado-peer.h"
#include "deliberate.h"
#include <QDateTime>
#include <QDebug>

using namespace deliberate;

namespace arado
{

PeerSweeper::PeerSweeper (DBManager *dbMgr, QObject *parent)
  :QObject (parent),
   dbm (dbMgr),
   sweepTimer (0),
   mainPeriod (45*60),    // each 45 minutes check is done
   maxAge (9*24*60*60)    // peers inactive longer than 9 days get deleted
{
  mainPeriod = Settings().value ("peersweep/period",mainPeriod).toInt();
  Settings().setValue ("peersweep/period",mainPeriod);
  maxAge = Settings().value ("peersweep/maxage",maxAge).toInt();
  Settings().setValue ("peersweep/maxage",maxAge);
  sweepTimer = new QTimer (this);
  connect (sweepTimer, SIGNAL (timeout()), this, SLOT (DoSweep()));
}

void
PeerSweeper::SetDB (DBManager * dbMgr)
{
  dbm = dbMgr;
}

void
PeerSweeper::SetPeriod (int secs)
{
  mainPeriod = secs;
  sweepTimer->setInterval (mainPeriod*1000);
}

void
PeerSweeper::SetMaxAge (qint64 secs)
{
  maxAge = secs;
}

int
PeerSweeper::Period ()
{
  return mainPeriod;
}

qint64
PeerSweeper::MaxAge ()
{
  return maxAge;
}

void
PeerSweeper::Start (int secs)
{
  if (secs > 0) {
    mainPeriod = secs;
  }
  sweepTimer->start (mainPeriod * 1000);
}

void
PeerSweeper::Stop ()
{
  sweepTimer->stop ();
}

void
PeerSweeper::SingleShot (int secs)
{
  QTimer::singleShot (secs * 1000, this, SLOT (DoSweep()));
}

void
PeerSweeper::DoSweep ()
{
  qDebug () << " PeerSweeper DoSweep max age " << maxAge;
  if (dbm == 0) {
    return;
  }
  AradoPeerList  peerList = dbm->GetPeers ("C");
  int now = QDateTime::currentDateTime().toTime_t();
  for (int p=0; p<peerList.count(); p++) {
    QString nick = peerList.at(p).Nick();
    qDebug () << " PeerSweeper checking " << nick;
    qint64 timestamp;
    bool isKnown = dbm->GetPeerTime (nick, timestamp);
    if (isKnown) {
      dbm->MarkPeerTime (nick, now);
    } else {
      qint64 age = now - timestamp;
      qDebug () << " PeerSweeper seing age " << age;
      if (age > maxAge) {
        qDebug () << " PeerSweeper removing " << nick;
        dbm->RemovePeerS (nick);
      }
    }
  }
}

} // namespace
