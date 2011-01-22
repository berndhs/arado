

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
#include "url-display-webview.h"
#include "db-manager.h"
#include "arado-url.h"
#include "search.h"
#include "deliberate.h"
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
#include <QDebug>
#include <QUrl>
#include <QKeySequence>


using namespace deliberate;

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
   slowTimer (0),
   refreshPeriod (18),
   autoRefresh (false),
   maxUrlsShown (150)
{
  search = new Search (this);
  ui.setupUi (this);

  urlDisplayView = NULL;
  LoadDisplay ();
  browseIcon = QIcon (":/images/kugar.png");
  connect (ui.addUrlButton, SIGNAL (clicked()),
           this, SLOT (AddButton ()));
  connect (ui.recentButton, SIGNAL (clicked ()),
           this, SLOT (RecentButton ()));
  connect (ui.searchButton, SIGNAL (clicked ()),
           this, SLOT (DoSearch ()));
  connect (ui.findHashButton, SIGNAL (clicked()),
           this, SLOT (DoHashLookup ()));
  connect (ui.autoCheck, SIGNAL (stateChanged(int)),
           this, SLOT (ChangedAutoRefresh (int)));
  ui.searchButton->setShortcut(QKeySequence(Qt::Key_Return));
  connect (search, SIGNAL (Ready (int)), this, SLOT (GetSearchResult (int)));
  refreshUrls = new QTimer (this);
  connect (refreshUrls, SIGNAL (timeout()), this, SLOT (Refresh()));

  slowTimer = new QTimer (this);
  connect (slowTimer, SIGNAL (timeout()), this, SLOT (SlowUpdate()));

  refreshPeriod = Settings ().value ("urldisplay/refreshperiod",
                                     refreshPeriod).toInt();
  Settings().setValue ("urldisplay/refreshperiod",refreshPeriod);
  autoRefresh = Settings ().value ("urldisplay/autorefresh",
                                   autoRefresh).toBool();
  Settings().setValue ("urldisplay/autorefresh",autoRefresh);
  ui.autoCheck->setChecked (autoRefresh);
  if (autoRefresh) {
    refreshUrls->start (refreshPeriod*1000);
  } else {
    refreshUrls->stop ();
  }

  int slowPeriod (5*60);
  slowPeriod = Settings ().value ("urldisplay/slowperiod",
                                  slowPeriod).toInt();
  Settings().setValue ("urldisplay/slowperiod",slowPeriod);
  slowTimer->start (slowPeriod*1000);
  QTimer::singleShot (10*1000, this, SLOT (SlowUpdate()));

  maxUrlsShown = Settings().value ("urldisplay/maxshown",
                                   maxUrlsShown).toInt();
  Settings().setValue ("urldisplay/maxshown",maxUrlsShown);
}

void
UrlDisplay::SetDB (DBManager *dbm)
{
  db = dbm;
  if (search) {
    search->SetDB (db);
  }
}

bool
UrlDisplay::AutoRefreshOn ()
{
  return autoRefresh;
}

void
UrlDisplay::ChangedAutoRefresh (int newstate)
{
  autoRefresh = ui.autoCheck->isChecked ();
  Settings().setValue ("urldisplay/autorefresh",autoRefresh);
  if (autoRefresh) {
    refreshPeriod = 18;
    refreshPeriod = Settings ().value ("urldisplay/refreshperiod",
                                     refreshPeriod).toInt();
    Settings().setValue ("urldisplay/refreshperiod",refreshPeriod);
    refreshUrls->start (refreshPeriod*1000);
  } else {
    refreshUrls->stop ();
  }
  Settings().sync ();
}

void
UrlDisplay::AddButton ()
{
  emit AddUrl (ui.textInput->text());
}

void
UrlDisplay::RecentButton ()
{
  urlDisplayView->Updating();
  ui.bottomLabel->setText (tr("Updating .........."));
  if (db) {
    db->ResetUrlAddCount ();
  }
  QTimer::singleShot (50,this, SLOT (Refresh ()));
}

void
UrlDisplay::Refresh (bool whenHidden)
{
  maxUrlsShown = Settings().value ("urldisplay/maxshown",
                                   maxUrlsShown).toInt();
  ShowRecent (maxUrlsShown, whenHidden);
  urlDisplayView->UpdatingFinished();
  if (refreshUrls) {
    if (autoRefresh) {
      refreshUrls->start (refreshPeriod*1000);
    } else {
      refreshUrls->stop ();
    }
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
  urlDisplayView->ShowUrls(urls);

  ShowAddCount ();
}

void
UrlDisplay::ShowAddCount ()
{
  int urlAddCount (0);
  if (db) {
    urlAddCount = db->UrlsAdded();
  };
  QString countText;
  if (urlAddCount > 0) {
    countText = tr("Show Recent: %1")
                .arg (urlAddCount);
  } else {
    countText = tr("Show Recent");
  }
  ui.recentButton->setText (countText);
}

void
UrlDisplay::UrlsAdded (int numAdded)
{
  QString labelText = tr("Last update at %1")
                      .arg(QDateTime::currentDateTime()
                           .toString("hh:mm:ss"));
  ui.bottomLabel->setText (labelText);
  if (numAdded > 0) {
    ShowAddCount ();
  }
}

void
UrlDisplay::ShowRecent (int howmany, bool whenHidden)
{
  if (db && !locked && (whenHidden || isVisible ())) {
    AradoUrlList urls = db->GetRecent (howmany);
    ShowUrls (urls);
    urlDisplayView->SortByTime(Qt::DescendingOrder);
    QString labelTime = QDateTime::currentDateTime ().toString(Qt::ISODate);
    labelTime.replace ('T'," ");
    QString labelText (tr("Recent Consciousness to %1")
                       .arg (labelTime));
    SetStatusMessage (labelText);
  }
}

void
UrlDisplay::DoSearch ()
{
  searchId = -1;
  if (search) {
    urlDisplayView->ClearContents();
    searchData = ui.textInput->text();
    searchId = search->Liberal (searchData);
    ui.textInput->setStyleSheet("background-color: black; border: 2px solid black");
    ui.bottomLabel->setText (tr("Searching ... %1 ............").arg (searchData));
  }
}

void
UrlDisplay::DoHashLookup ()
{
  searchId = -1;
  if (search) {
    urlDisplayView->ClearContents();
    searchData = ui.textInput->text();
    searchId = search->Hash (searchData);
    ui.textInput->setStyleSheet("background-color: black; border: 2px solid black");
    ui.bottomLabel->setText (tr("Searching ... %1 ............").arg (searchData));
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
    urlDisplayView->ClearSelection();
    ShowUrls (urls);
    ui.textInput->setStyleSheet("background-color: white; border: 2px solid #079E00;");
    ui.bottomLabel->setText (tr("Search Experience for: %1").arg (searchData));
  }
}

void
UrlDisplay::SlowUpdate ()
{
  qint64 numUrls = db->NumUrls();
  ui.middleLabel->setText (tr("Search within about %1 entries.").arg(numUrls));
}

void
UrlDisplay::SetStatusMessage (const QString & msg)
{
  ui.bottomLabel->setText (msg);
}

void
UrlDisplay::LoadDisplay ()
{
  if (urlDisplayView) {
    delete urlDisplayView;
    urlDisplayView = 0;
  }

  urlDisplayView = new UrlDisplayWebView();
 
  ui.urlDisplayLayout->addWidget(urlDisplayView);
  urlDisplayView->setSizePolicy(QSizePolicy::Expanding, 
                                QSizePolicy::Expanding);
}

} // namespace

