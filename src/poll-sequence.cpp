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
   timerA (this),
   timerB (this),
   timerC (this)
{
  ComputePeriods (1,1,1);
  FindRatios ();
  connect (&timerA, SIGNAL (timeout()), this, SLOT (PollA()));
  connect (&timerB, SIGNAL (timeout()), this, SLOT (PollB()));
  connect (&timerC, SIGNAL (timeout()), this, SLOT (PollC()));
}

void
PollSequence::PollClient(HttpClient *httpClient, bool force)
{
  client = httpClient;
qDebug () << " PollSequence::PollClient db " << db;
  if (db) {
    AradoPeerList list = db->GetPeers ("A");
    AradoPeerList::const_iterator ait;
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksA.insert (ait->Nick());
    }
    list = db->GetPeers ("B");
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksB.insert (ait->Nick());
    }
    list = db->GetPeers ("C");
    for (ait = list.constBegin(); ait != list.constEnd(); ait++) {
      nicksC.insert (ait->Nick());
    }
    ComputePeriods (nicksA.count(), nicksB.count(), nicksC.count());
    if (force && httpClient) {
      QStringList bigList;
      bigList << nicksA.toList() 
              << nicksB.toList()
              << nicksC.toList();
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
  ComputePeriods (nicksA.count(), nicksB.count(), nicksC.count());   
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
  ComputePeriods (nicksA.count(), nicksB.count(), nicksC.count());
  RestartTimers ();
}

void
PollSequence::PollA ()
{
  qDebug () << " POLL A set size " << nicksA.size();
  Poll (nicksA, pollItA, maxRecent["A"], maxRandom["A"]);
}

void
PollSequence::PollB ()
{
  qDebug () << " POLL B set size " << nicksB.size();
  Poll (nicksB, pollItB, maxRecent["B"], maxRandom["B"]);
}

void
PollSequence::PollC ()
{
  qDebug () << " POLL C set size " << nicksC.size();
  Poll (nicksC, pollItC, maxRecent["C"], maxRandom["C"]);
}

void
PollSequence::Poll (QSet <QString> & nickSet, 
                    QSet <QString>::iterator & nickIt,
                    int numRecent, int numRandom)
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
      client->PollPeer (*nickIt, numRecent, numRandom);
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
PollSequence::ComputePeriods (int countA, int countB, int countC)
{
  const qint64 msPerHour (60*60*1000);
  if (countA < 1) { countA = 1; }
  if (countB < 1) { countB = 1; }
  if (countC < 1) { countC = 1; }
  periodA = (msPerHour / urlFreqA) / countA;
  periodB = (msPerHour / urlFreqB) / countB;
  periodC = (msPerHour / urlFreqC) / countC;  
  qDebug () << " poll periods A " << periodA 
            << " B " << periodB << " C " << periodC;
}

void
PollSequence::FindRatios ()
{
  FindRatio ("A");
  FindRatio ("B");
  FindRatio ("C");
}

void
PollSequence::SaveRatios ()
{
  SaveRatio ("A");
  SaveRatio ("B");
  SaveRatio ("C");
}

void
PollSequence::SaveRatio (QString level)
{
  Settings().setSimpleValue (QString("traffic/maxrecent%1").arg(level),
                          maxRecent[level]);
  Settings().setSimpleValue (QString("traffic/maxrandom%1").arg(level),
                          maxRandom[level]);
}

void
PollSequence::FindRatio (QString level)
{
  int maxRec (50);
  int maxRand (50);
  maxRec = Settings().simpleValue (QString("traffic/maxrecent%1").arg(level),
                                  maxRec).toInt();
  maxRand = Settings().simpleValue (QString("traffic/maxrandom%1").arg(level),
                                  maxRand).toInt();
  maxRecent[level] = maxRec;
  maxRandom[level] = maxRand;
}

void
PollSequence::RefreshParams ()
{
  urlFreqA = Settings().simpleValue ("traffic/urlFrequencyA",urlFreqA).toDouble();
  urlFreqB = Settings().simpleValue ("traffic/urlFrequencyB",urlFreqB).toDouble();
  urlFreqC = Settings().simpleValue ("traffic/urlFrequencyC",urlFreqC).toDouble();
  FindRatios ();
}

void
PollSequence::SaveParams ()
{
  Settings().setSimpleValue ("traffic/urlFrequencyA",urlFreqA);
  Settings().setSimpleValue ("traffic/urlFrequencyB",urlFreqB);
  Settings().setSimpleValue ("traffic/urlFrequencyC",urlFreqC);
  SaveRatios ();
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

