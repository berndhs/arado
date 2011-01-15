#ifndef URLDISPLAYWEBWIDGET_H
#define URLDISPLAYWEBWIDGET_H


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


#include "url-display-view.h"
#include <QMap>
#include <QList>

class QUrl;

namespace Ui
{
class UrlDisplayWebView;
}

namespace arado
{

class UrlDisplayWebViewTab;

class UrlDisplayWebView : public UrlDisplayView
{
  Q_OBJECT

public:
  explicit UrlDisplayWebView(QWidget *parent = 0);
  ~UrlDisplayWebView();

  void ShowUrls(QList<AradoUrl> & urls);
  void Updating() {}
  void UpdatingFinished() {}
  void ClearContents() {
    ClearItemsList();
  }
  void ClearSelection() {}
  void ClearItemsList();
  void SortByTime(Qt::SortOrder) {}

private slots:
  void UrlViewLinkClicked(const QUrl &);
  void TabNameChanged(UrlDisplayWebViewTab *, const QString &);
  void CloseTab(UrlDisplayWebViewTab *);

private:
  QString Html();
  QString HtmlBase();
  void BrowseUrl (const QUrl & url);

  void Load(const QList<arado::AradoUrl> &);

  QList<UrlDisplayViewItem *> items;
  QMap <QString, AradoUrl>    urlMap;
  Ui::UrlDisplayWebView *ui;
};

}

#endif // URLDISPLAYWEBWIDGET_H
