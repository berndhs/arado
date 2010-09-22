

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
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>
#include <QCursor>
#include <QDebug>

namespace arado
{

UrlDisplay::UrlDisplay (QWidget * parent)
  :QWidget (parent),
   db (0)
{
  ui.setupUi (this);
  connect (ui.urlTable, SIGNAL (itemActivated (QTableWidgetItem *)),
           this, SLOT (Picked (QTableWidgetItem*)));
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
  if (db && isVisible ()) {
    AradoUrlList urls = db->GetRecent (howmany);
    ui.urlTable->clearContents ();
    ui.urlTable->setRowCount (urls.size());
    ui.urlTable->setEditTriggers (0);
    for (int u=0; u<urls.size(); u++) {
      bool allowSort = ui.urlTable->isSortingEnabled ();
      ui.urlTable->setSortingEnabled (false);
      quint64 stamp;
      AradoUrl url = urls[u];
      stamp = url.Timestamp ();
qDebug () << " url " << url.Url() << " keys " << url.Keywords();
      QTableWidgetItem * item = new QTableWidgetItem (QString(url.Hash()));
      item->setData (Url_Celltype, Cell_Hash);
      item->setToolTip (tr("SHA1 hash of the Url"));
      ui.urlTable->setItem (u,0,item);

      item = new QTableWidgetItem (url.Description ());
      item->setData (Url_Celltype, Cell_Desc);
      item->setData (Url_Keywords, url.Keywords ());
      QString words = url.Keywords().join("\n");
      if (words.length() < 1) {
        words = tr ("no keywords");
      }
      item->setToolTip (words);
      ui.urlTable->setItem (u,1,item);

      item = new QTableWidgetItem (url.Url().toString());
      item->setData (Url_Celltype, Cell_Url);
      ui.urlTable->setItem (u,2,item);

      QString time = QDateTime::fromTime_t (stamp).toString(Qt::ISODate);
      time.replace ('T'," ");
      item = new QTableWidgetItem (time);
      item->setData (Url_Celltype, Cell_Time);
      ui.urlTable->setItem (u,3,item);
      ui.urlTable->setSortingEnabled (allowSort);
    }
  }
}

void
UrlDisplay::Picked (QTableWidgetItem *item)
{
  if (item) {
    CellType  tipo;
    tipo = CellType (item->data(Url_Celltype).toInt());
    if (tipo == Cell_Desc) {
    }
  }
}

} // namespace

