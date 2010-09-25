

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
#include <QDesktopServices>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QAction>
#include <QDebug>

namespace arado
{

UrlDisplay::UrlDisplay (QWidget * parent)
  :QWidget (parent),
   db (0),
   allowSort (false),
   locked (false)
{
  ui.setupUi (this);
  allowSort = ui.urlTable->isSortingEnabled ();
  connect (ui.urlTable, SIGNAL (itemDoubleClicked (QTableWidgetItem *)),
           this, SLOT (Picked (QTableWidgetItem*)));
  connect (ui.addUrlButton, SIGNAL (clicked()),
           this, SLOT (AddButton ()));
  connect (ui.openUrlButton, SIGNAL (clicked ()),
           this, SLOT (OpenUrl ()));
  connect (ui.recentButton, SIGNAL (clicked ()),
           this, SLOT (Refresh ()));
}

void
UrlDisplay::AddButton ()
{
  emit AddUrl (ui.textInput->text());
}

void
UrlDisplay::Refresh (bool whenHidden)
{
  ShowRecent (100, whenHidden);
}

void
UrlDisplay::Lock ()
{
  locked = true;
}

void
UrlDisplay::Unlock ()
{
  locked = false;
}

void
UrlDisplay::ShowRecent (int howmany, bool whenHidden)
{
  if (db && !locked && (whenHidden || isVisible ())) {
    AradoUrlList urls = db->GetRecent (howmany);
    ui.urlTable->clearContents ();
    ui.urlTable->setRowCount (urls.size());
    ui.urlTable->setEditTriggers (0);
    for (int u=0; u<urls.size(); u++) {
      quint64 stamp;
      AradoUrl url = urls[u];
      stamp = url.Timestamp ();
      Lock ();
      ui.urlTable->setSortingEnabled (false);
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
      Unlock ();
      ui.urlTable->setSortingEnabled (allowSort);
      QString labelTime = QDateTime::currentDateTime ().toString(Qt::ISODate);
      labelTime.replace ('T'," ");
      QString labelText (tr("Recent to %1").arg (labelTime));
      ui.bottomLabel->setText (labelText);
    }
  }
}

void
UrlDisplay::OpenUrl ()
{
  QTableWidgetItem * current = ui.urlTable->currentItem ();
  if (current) {
    if (CellType (current->data(Url_Celltype).toInt()) 
        == Cell_Url) {
      QUrl target (current->text ());    
      QDesktopServices::openUrl (target);
    }
  }
}


void
UrlDisplay::Picked (QTableWidgetItem *item)
{
  if (item) {
    Lock ();
    CellType  tipo;
    tipo = CellType (item->data(Url_Celltype).toInt());
    if (tipo == Cell_Url) {
      CellMenuUrl (item);
    } else if (tipo == Cell_Desc) {
      CellMenuDesc (item);
    } else {
      CellMenu (item);
    } 
    Unlock ();
  }
}


QAction *
UrlDisplay::CellMenu (const QTableWidgetItem *item,
                      const QList<QAction *>  extraActions)
{
  if (item == 0) {
    return 0;
  }
  QMenu menu (this);
  QAction * copyAction = new QAction (tr("Copy Text"),this);
  QAction * mailAction = new QAction (tr("Mail Text"),this);
  menu.addAction (copyAction);
  menu.addAction (mailAction);
  if (extraActions.size() > 0) {
    menu.addSeparator ();
  }
  for (int a=0; a < extraActions.size(); a++) {
    menu.addAction (extraActions.at (a));
  }
  
  QAction * select = menu.exec (QCursor::pos());
  if (select == copyAction) {
    QClipboard *clip = QApplication::clipboard ();
    if (clip) {
      clip->setText (item->text());  
    }
    return 0;
  } else if (select == mailAction) {
    QString mailBody = item->text();
    QString urltext = tr("mailto:?subject=Arado\%20Data&body=%1")
                      .arg (mailBody);
    QDesktopServices::openUrl (urltext);
    return 0;
  } else {
    return select;
  }
}

void
UrlDisplay::CellMenuUrl (const QTableWidgetItem * item)
{
  if (item == 0) {
    return;
  }
  QAction * openAction = new QAction (tr("Browse URL"),this);
  QList<QAction*> list;
  list.append (openAction);

  QAction * select = CellMenu (item, list);
  if (select == openAction) {
    QUrl url (item->text());
    QDesktopServices::openUrl (url);
  }
}


void
UrlDisplay::CellMenuDesc (const QTableWidgetItem * item)
{
  if (item == 0) {
    return;
  }
  QAction *copyKeysAction = new QAction (tr("Copy Keywords"), this);
  QAction *mailKeysAction = new QAction (tr("Mail Keywords"), this);
  QList <QAction*> list;
  list.append (copyKeysAction);
  list.append (mailKeysAction);

  QAction * select = CellMenu (item, list);
  QString mailBody;
  bool    mailit (false);
  if (select == copyKeysAction) {
    QClipboard * clip = QApplication::clipboard ();
    if (clip) {
      QString keytext = item->data (Url_Keywords).toStringList().join("\n");
      clip->setText (keytext);
    }
  } else if (select == mailKeysAction) {
    mailBody = item->data (Url_Keywords).toStringList().join("\n");
    mailit = (mailBody.length() > 0);
  }
  if (mailit) {
    QString urltext = tr("mailto:?subject=Arado\%20Data&body=%1")
                      .arg (mailBody);
    QDesktopServices::openUrl (urltext);
  }
}


} // namespace

