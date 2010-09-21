

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, 
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

#include "url-display.h"
#include "db-manager.h"
#include "arado-url.h"
#include <QTableWidgetItem>
#include <QDateTime>
#include <QDebug>

namespace arado
{

UrlDisplay::UrlDisplay (QWidget * parent)
  :QWidget (parent),
   db (0)
{
  ui.setupUi (this);
}

void
UrlDisplay::Refresh ()
{
  qDebug () << " Refresh " << QDateTime::currentDateTime ();
  ShowRecent (10);
}

void
UrlDisplay::ShowRecent (int howmany)
{
  if (db) {
    StampedUrlList urls = db->GetRecent (howmany);
    ui.urlTable->clearContents ();
    ui.urlTable->setRowCount (urls.size());
    for (int u=0; u<urls.size(); u++) {
      quint64 stamp = urls[u].first;
      AradoUrl url = urls[u].second;
      QTableWidgetItem * item = new QTableWidgetItem (QString(url.Hash()));
      ui.urlTable->setItem (u,0,item);
      item = new QTableWidgetItem (url.Description ());
      ui.urlTable->setItem (u,1,item);
      item = new QTableWidgetItem (url.Url().toString());
      ui.urlTable->setItem (u,2,item);
      item = new QTableWidgetItem (QDateTime::fromTime_t(stamp).toString());
      ui.urlTable->setItem (u,3,item);
    }
  }
}

} // namespace

