
#include "rss-list.h"

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

#include "arado-feed.h"
#include "db-manager.h"
#include <QTableWidgetItem>
#include <QList>
#include <QSet>
#include <QDebug>

namespace arado
{

RssList::RssList (QWidget *parent)
  :QWidget (parent),
   dbm (0),
   changedSomething (false)
{
  ui.setupUi (this);
  hide ();
  Connect ();
}

void
RssList::Show ()
{
  ListFeeds ();
  show ();
}

void
RssList::Hide ()
{
  hide ();
}

void
RssList::Connect ()
{
  connect (ui.newButton, SIGNAL (clicked()), this, SLOT (DoAdd()));
  connect (ui.deleteButton, SIGNAL (clicked()), this, SLOT (DoDelete()));
  connect (ui.closeButton1, SIGNAL (clicked()), this, SLOT (DoClose()));
  connect (ui.closeButton2, SIGNAL (clicked()), this, SLOT (DoClose()));
  connect (ui.saveButton, SIGNAL (clicked()), this, SLOT (DoSave()));

  /* Hide Advanced gui */
  rsseditadvancedview(false);
  connect(ui.rsseditadvancedview, SIGNAL(toggled(bool)), this, SLOT(rsseditadvancedview(bool)));
  //

}

void
RssList::DoClose ()
{
  ui.feedTable->clearContents ();
  Hide ();
  emit Closed (changedSomething);
  changedSomething = false;
}

void
RssList::DoAdd ()
{
  qDebug () << "RssList  DoAdd";
  int newrow = ui.feedTable->rowCount();
qDebug () << " RssList add Item row " << newrow;
  ui.feedTable->setRowCount (newrow+1);
  QTableWidgetItem * nickItem = new QTableWidgetItem (tr("New Feed"));
  ui.feedTable->setItem (newrow, 0, nickItem);
  QTableWidgetItem * urlItem = new QTableWidgetItem (tr("http://"));
  ui.feedTable->setItem (newrow, 1, urlItem);
  ui.feedTable->scrollToBottom ();
  ui.feedTable->editItem (nickItem);
}

void
RssList::DoDelete ()
{
  qDebug () << "RssList   DoDelete";
  if (dbm == 0) {
    return;
  }
  QSet <int> removeRows;
  QList <QTableWidgetItem*> removeItems = ui.feedTable->selectedItems();
  for (int i=0; i<removeItems.size(); i++) {
    removeRows.insert (removeItems.at(i)->row());
  }
  QSet<int>::iterator rit = removeRows.begin();
  while (rit != removeRows.end()) {
    int row = *rit;
    QTableWidgetItem * nickItem = ui.feedTable->item (row,0);
    if (nickItem) {
      dbm->RemoveFeed (nickItem->text());
    }
    rit++;
  }
  ListFeeds ();
  changedSomething = true;
}

void
RssList::DoSave ()
{
  qDebug () << "RssList  DoSave";
  if (dbm == 0) {
    return;
  }
  QSet <int> saveRows;
  QList <QTableWidgetItem*> saveItems = ui.feedTable->selectedItems();
  for (int i=0; i<saveItems.size(); i++) {
    saveRows.insert (saveItems.at(i)->row());
  }
  QSet<int>::iterator sit = saveRows.begin();
  while (sit != saveRows.end()) {
    int row = *sit;
    QTableWidgetItem * nickItem = ui.feedTable->item (row,0);
    QTableWidgetItem * urlItem = ui.feedTable->item (row,1);
    if (nickItem && urlItem) {
      dbm->WriteFeed (nickItem->text(),
                      QUrl (urlItem->text()));
    }
    sit++;
  }
  ui.feedTable->clearSelection ();
  changedSomething = true;
}

void
RssList::ListFeeds ()
{
qDebug () << "RssList List Feeds dbm " << dbm;
  AradoFeedList feeds;
  if (dbm) {
    ui.feedTable->clearContents ();
    ui.feedTable->setRowCount (0);
    feeds = dbm->GetFeeds ();
    int nf = feeds.size();
    for (int f=0; f<nf; f++) {
      ListFeed (feeds.at(f));
    }
  }
}

void
RssList::ListFeed (const AradoFeed & feed)
{
  int newrow = ui.feedTable->rowCount();
qDebug () << " RssList add Item row " << newrow;
  ui.feedTable->setRowCount (newrow+1);
  QTableWidgetItem * nickItem = new QTableWidgetItem (feed.Nick());
  ui.feedTable->setItem (newrow, 0, nickItem);
  QTableWidgetItem * urlItem = new QTableWidgetItem (feed.Url().toString());
  ui.feedTable->setItem (newrow, 1, urlItem);
}

void RssList::rsseditadvancedview(bool show)
{
  ui.saveButton->setVisible(show);
  ui.closeButton2->setVisible(show);
  ui.deleteButton->setVisible(show);
  ui.newButton->setVisible(show);
  ui.feedTable->setVisible(show);
}

} // namespace
