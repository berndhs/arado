/** PollSequence
  */

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

#include "poll-sequence.h"
#include "db-manager.h"
#include "deliberate.h"
#include <QDebug>

using namespace deliberate;

namespace arado
{

PollSequence::PollSequence (QObject * parent)
  :QObject (parent),
   db (0),
   client (0),
   urlFreqA (600.0),
   urlFreqB (60.0),
   urlFreqC (40.0),
   periodA (0),
   periodB (0),
   periodC (0),
   urlChunkA (60),
   urlChunkB (40),
   urlChunkC (20),
   timerA (this),
   timerB (this),
   timerC (this)
{
  ComputePeriods ();
  connect (&timerA, SIGNAL (timeout()), this, SLOT (PollA()));
  connect (&timerB, SIGNAL (timeout()), this, SLOT (PollB()));
  connect (&timerC, SIGNAL (timeout()), this, SLOT (PollC()));
}

void
PollSequence::PollClient(HttpClient *httpClient, bool force)
{
  client = httpClient;
  if (db) {
    AradoPeerList list = db->GetPeers ("A");
    QStringList bigList;
    AradoPeerList::const_iterator ait;
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksA.insert (ait->Nick());
      if (force) {
        bigList.append (ait->Nick());
      }
    }
    list = db->GetPeers ("B");
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksB.insert (ait->Nick());
      if (force) {
        bigList.append (ait->Nick());
      }
    }
    list = db->GetPeers ("C");
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksC.insert (ait->Nick());
      if (force) {
        bigList.append (ait->Nick());
      }
    }
    if (force && httpClient) {
      httpClient->PollPeers (bigList);  
    }
  }
}

void
PollSequence::Start ()
{ 
  qDebug () << " Sequence Start";
  RefreshParams ();
  SaveParams ();
  ComputePeriods ();   
  RestartTimers ();  
  pollItA = nicksA.begin();
  pollItB = nicksB.begin();
  pollItC = nicksC.begin();             
}

void
PollSequence::Restart ()
{
  qDebug () << " Sequence Restart";
  RefreshParams ();
  SaveParams ();
  ComputePeriods ();
  RestartTimers ();
}

void
PollSequence::PollA ()
{
  qDebug () << " POLL A set size " << nicksA.size();
  Poll (nicksA, pollItA);
}

void
PollSequence::PollB ()
{
  qDebug () << " POLL B set size " << nicksB.size();
  Poll (nicksB, pollItB);
}

void
PollSequence::PollC ()
{
  qDebug () << " POLL C set size " << nicksC.size();
  Poll (nicksC, pollItC);
}

void
PollSequence::Poll (QSet <QString> & nickSet, 
                    QSet <QString>::iterator & nickIt)
{
  if (nickSet.size() < 1) {
    return;
  }
  if (nickIt == nickSet.end()) {
    nickIt = nickSet.begin();
  } 
  if (nickIt != nickSet.end()) {
    if (client) {
      qDebug () << " Sequencer polling " << *nickIt;
      client->PollPeer (*nickIt);
    }
  }
  nickIt ++;
}

void
PollSequence::Stop ()
{
  timerA.stop ();
  timerB.stop ();
  timerC.stop ();
}

void
PollSequence::ComputePeriods ()
{
  qint64 msPerHour (60*60*1000);
  periodA = msPerHour / urlFreqA;
  periodB = msPerHour / urlFreqB;
  periodC = msPerHour / urlFreqC;  
  qDebug () << " poll periods A " << periodA 
            << " B " << periodB << " C " << periodC;
}

void
PollSequence::RefreshParams ()
{
  urlFreqA = Settings().value ("traffic/urlFrequencyA",urlFreqA).toDouble();
  urlFreqB = Settings().value ("traffic/urlFrequencyB",urlFreqB).toDouble();
  urlFreqC = Settings().value ("traffic/urlFrequencyC",urlFreqC).toDouble();
  urlChunkA = Settings().value ("traffic/urlChunkA",urlChunkA).toInt ();
  urlChunkB = Settings().value ("traffic/urlChunkB",urlChunkB).toInt ();
  urlChunkC = Settings().value ("traffic/urlChunkC",urlChunkC).toInt ();
}

void
PollSequence::SaveParams ()
{
  Settings().setValue ("traffic/urlFrequencyA",urlFreqA);
  Settings().setValue ("traffic/urlFrequencyB",urlFreqB);
  Settings().setValue ("traffic/urlFrequencyC",urlFreqC);
  Settings().setValue ("traffic/urlChunkA",urlChunkA);
  Settings().setValue ("traffic/urlChunkB",urlChunkB);
  Settings().setValue ("traffic/urlChunkC",urlChunkC);
  Settings().sync();    
}

void
PollSequence::RestartTimers ()
{ 
  if (periodA > 0) {
    timerA.start (periodA);
  }
  if (periodB > 0) {
    timerB.start (periodB);
  }
  if (periodC > 0) {
    timerC.start (periodC);
  }    
}

} // namespace

