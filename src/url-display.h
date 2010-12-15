#ifndef URL_DISPLAY_H
#define URL_DISPLAY_H


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
#include <Qt>
#include "ui_url-display.h"
#include "arado-url.h"

class QTableWidgetItem;
class QAction;
class QEvent;
class QTimer;
class QTextBrowser;
class QResizeEvent;

namespace arado
{

class DBManager;
class Search ;

class UrlDisplay : public QWidget 
{
Q_OBJECT

public:

  enum ModelData {
       Url_Keywords = Qt::UserRole + 1,
       Url_Celltype = Qt::UserRole + 2
       };
  enum UrlCellType {
       Cell_None = 0,
       Cell_Hash = 1,
       Cell_Desc = 2,
       Cell_Url = 3,
       Cell_Time = 4,
       Cell_Browse = 5
       };

  UrlDisplay (QWidget *parent);
  
  void  SetDB (DBManager *dbm);

  void  ShowRecent (int howmany, bool whenHidden = false);
  void  SetStatusMessage (const QString &msg);

public slots:

  void  Refresh (bool whenHidden = false);
  void  Picked (QTableWidgetItem *item);
  void  AddButton ();
  void  DoOpenUrl (QTableWidgetItem * urlItem = 0);
  void  DoSearch ();
  void  GetSearchResult (int resultid);
  void  UrlsAdded (int numAdded);
  void  ShowAddCount ();

private slots:

  void ActiveCell (int row, int col, int oldRow, int oldCol);

signals:

  void AddUrl (QString urlString);

protected:
 
  void resizeEvent ( QResizeEvent * event );


private:

  void     Lock ();
  void     Unlock ();
  void     ShowUrls (AradoUrlList & urls);
  void     CellMenuUrl (const QTableWidgetItem * item);
  void     CellMenuDesc (const QTableWidgetItem * item);
  void     CellMenuTime (const QTableWidgetItem * item);
  QAction* CellMenu (const QTableWidgetItem *item,
                    const QList <QAction*> extraActions
                          = QList <QAction*>() );

  Ui_UrlDisplay     ui;
  DBManager        *db;
  Search           *search;
  bool              allowSort;
  bool              locked;
  int               searchId;
  QString           searchData;
  QTimer           *refreshUrls;
  int               refreshPeriod;
  int               normalRowHeight;
  int               bigRowHeight;
  QIcon             browseIcon;

  static const int Col_Mark;
  static const int Col_Title;
  static const int Col_Url;
  static const int Col_Time;
  static const int Col_Browse;

};

} // namespace

#endif
