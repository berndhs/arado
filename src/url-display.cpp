

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

#include "url-display.h"
#include "db-manager.h"
#include "arado-url.h"
#include "search.h"
#include <QtGlobal>
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
#include <QTextBrowser>
#include <QLineEdit>
#include <QSizePolicy>
#include <QAction>
#include <QTimer>
#include <QResizeEvent>
#include <QDebug>

namespace arado
{

UrlDisplay::UrlDisplay (QWidget * parent)
  :QWidget (parent),
   db (0),
   search (0),
   allowSort (false),
   locked (false),
   searchId (-1),
   refreshUrls (0),
   refreshPeriod (30000)
{
  search = new Search (this);
  ui.setupUi (this);
  //ui.fakeButton->setEnabled (false);
  browseIcon = QIcon (":/images/kugar.png");
  allowSort = ui.urlTable->isSortingEnabled ();
  connect (ui.urlTable, SIGNAL (currentCellChanged (int, int, int, int)),
           this, SLOT (ActiveCell (int, int, int, int)));
  connect (ui.urlTable, SIGNAL (itemDoubleClicked (QTableWidgetItem *)),
           this, SLOT (Picked (QTableWidgetItem*)));
  connect (ui.addUrlButton, SIGNAL (clicked()),
           this, SLOT (AddButton ()));
  connect (ui.recentButton, SIGNAL (clicked ()),
           this, SLOT (Refresh ()));
  connect (ui.searchButton, SIGNAL (clicked ()),
           this, SLOT (DoSearch ()));
  connect (search, SIGNAL (Ready (int)), this, SLOT (GetSearchResult (int)));
  refreshUrls = new QTimer (this);
  connect (refreshUrls, SIGNAL (timeout()), this, SLOT (Refresh()));
  refreshUrls->start (refreshPeriod);
}

void
UrlDisplay::SetDB (DBManager *dbm)
{
  db = dbm;
  if (search) {
    search->SetDB (db);
  }
}

void
UrlDisplay::AddButton ()
{
  emit AddUrl (ui.textInput->text());
}

void
UrlDisplay::Refresh (bool whenHidden)
{
  ShowRecent (1000, whenHidden);
  if (refreshUrls && !refreshUrls->isActive()) {
    refreshUrls->start (refreshPeriod);
  }
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
UrlDisplay::ShowUrls (AradoUrlList & urls)
{
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
    item->setToolTip (tr("Arado-Flashmark"));
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
    item = new QTableWidgetItem (tr(""));
    item->setToolTip (tr("Browse"));
    item->setData (Url_Celltype, Cell_Browse);
    item->setIcon (browseIcon);
    ui.urlTable->setItem (u, 4, item);
    ui.bottomLabel->setText (labelText);
  }
  if (ui.urlTable->rowCount() > 0) {
    normalRowHeight = ui.urlTable->rowHeight (0);
    bigRowHeight = qMin (3*normalRowHeight, ui.urlTable->size().height());
  }
}

void
UrlDisplay::UrlsAdded (int numAdded)
{
  Q_UNUSED (numAdded)
  QString labelText = tr("Network update at %1")
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
  ui.bottomLabel->setText (labelText);
}

void
UrlDisplay::ShowRecent (int howmany, bool whenHidden)
{
  if (db && !locked && (whenHidden || isVisible ())) {
    AradoUrlList urls = db->GetRecent (howmany);
    ShowUrls (urls);
  }
}

void
UrlDisplay::ActiveCell (int row, int col, int oldRow, int oldCol)
{
  QTableWidgetItem * item = ui.urlTable->item (row, col);
  QTableWidgetItem * oldItem = ui.urlTable->item (oldRow, oldCol);
  if (oldItem) {
    ui.urlTable->setRowHeight (oldRow, normalRowHeight);
  }
  if (item) {
    UrlCellType ctype = UrlCellType (item->data (Url_Celltype).toInt());
    if (ctype == Cell_Desc) {
      ui.urlTable->setRowHeight (row, bigRowHeight);
    } else if (ctype == Cell_Browse) {
      QUrl url;
      QTableWidgetItem * urlItem (0);
      for (int c=0; c<ui.urlTable->columnCount(); c++) {
        urlItem = ui.urlTable->item (row, c);
        if (urlItem && 
            UrlCellType (urlItem->data (Url_Celltype).toInt()) 
             == Cell_Url) {
          DoOpenUrl (urlItem);
          break;
        }
      }
    }
  }
}

void
UrlDisplay::resizeEvent (QResizeEvent * event)
{
  if (event) {
    bigRowHeight = qMin (3*normalRowHeight, ui.urlTable->size().height());
  }
}

void
UrlDisplay::DoOpenUrl (QTableWidgetItem * urlItem)
{
  QTableWidgetItem * current (0);
  if (urlItem == 0) {
    current = ui.urlTable->currentItem ();
  } else {
    current = urlItem;
  }
  if (current) {
    if (UrlCellType (current->data(Url_Celltype).toInt()) 
        == Cell_Url) {
      QUrl target (current->text ());    
      QDesktopServices::openUrl (target);
    }
  }
}

void
UrlDisplay::DoSearch ()
{
  searchId = -1;
  if (search) {
    searchData = ui.textInput->text();
    searchId = search->ExactKeyword (searchData);
    ui.bottomLabel->setText (tr("Searching %1").arg (searchData));
  }
}

void
UrlDisplay::GetSearchResult (int resultId)
{
  if (db && search) {
    if (refreshUrls) {
      refreshUrls->stop ();
    }
    QStringList  hashList;
    search->ResultList (resultId, hashList);
    AradoUrlList urls;
    QStringList::const_iterator lit;
    AradoUrl url;
    bool haveit;
    for (lit = hashList.constBegin(); lit != hashList.constEnd(); ++lit) {
      haveit = db->ReadUrl (*lit, url);
      if (haveit) {
        urls.append (url);
      }
    }
    ShowUrls (urls);
    ui.bottomLabel->setText (tr("Search Results %1").arg (searchData));
  }
}


void
UrlDisplay::Picked (QTableWidgetItem *item)
{
  if (item) {
    Lock ();
    UrlCellType  tipo;
    tipo = UrlCellType (item->data(Url_Celltype).toInt());
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
  copyAction->setIcon(QPixmap(":/images/copy.png"));
  QAction * mailAction = new QAction (tr("Mail Text"),this);
  mailAction->setIcon(QPixmap(":/images/mail.png"));
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
    QStringList mailBodytotal;
    QString mailBody = item->text();
    QString blankline (tr("    "));
    QString webpageline (tr("Discover http://arado.sf.net Websearch - Syncs, shortens "
                             "and searches within (y)our URLs and Bookmarks."));
    mailBodytotal << mailBody
                  << blankline
                  << webpageline ;
    QString urltext = tr("mailto:?subject=Aradofied\%20Web\%20Alert\%20for\%20you&body=%1")
                      .arg (mailBodytotal.join("\r\n"));
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
  openAction->setIcon(QPixmap(":/images/kugar.png"));
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
  copyKeysAction->setIcon(QPixmap(":/images/copy.png"));
  QAction *mailKeysAction = new QAction (tr("Mail Keywords"), this);
  mailKeysAction->setIcon(QPixmap(":/images/mail.png"));
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

