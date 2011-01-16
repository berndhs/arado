#include "url-display-webview.h"



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


#include "ui_url-display-webview.h"

#include "url-display-viewitem.h"
#include "url-display-webviewtab.h"
#include "deliberate.h"
#include "arado-url.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDesktopServices>
#include <QMap>
#include <QList>
#include <QDebug>

using namespace deliberate;

namespace arado
{

UrlDisplayWebView::UrlDisplayWebView(QWidget *parent) :
  UrlDisplayView(parent),
  ui(new Ui::UrlDisplayWebView)
{
  ui->setupUi(this);

  ui->urlView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  ui->urlView->setContextMenuPolicy(Qt::NoContextMenu);
  QObject::connect(ui->urlView, SIGNAL(linkClicked(const QUrl &)),
                   this, SLOT(UrlViewLinkClicked(const QUrl &)));
  int iconHeight (16);
  int iconWidth (16);
  iconHeight = Settings().value ("urldisplay/aiconheight",iconHeight).toInt();
  Settings().setValue ("urldisplay/aiconheight",iconHeight);
  iconWidth = Settings().value ("urldisplay/aiconwidth",iconWidth).toInt();
  Settings().setValue ("urldisplay/aiconwidth",iconWidth);
}

UrlDisplayWebView::~UrlDisplayWebView()
{
  ClearItemsList();
  delete ui;
}

QString
UrlDisplayWebView::Html()
{
  QString itemsHtml;

  for (int i = 0; i < items.count(); ++i) {
    itemsHtml.append(items.at(i)->Html());
    //itemsHtml.append("<hr style=\"width:30%\">");
  }
  return HtmlBase().arg(itemsHtml);
}

QString
UrlDisplayWebView::HtmlBase()
{
  QFile file;
  QTextStream in(&file);
  in.setCodec("UTF-8");
  file.setFileName(":html/html/url-display-view.html");
  file.open(QIODevice::ReadOnly | QIODevice::Text);
  return in.readAll();
}

void
UrlDisplayWebView::ShowUrls (QList<arado::AradoUrl> & urls)
{
  ui->browserTabWidget->setCurrentIndex(0);
  Load(urls);
  ui->urlView->setHtml(Html(), QUrl(":html/"));
}

void
UrlDisplayWebView::Load(const QList<arado::AradoUrl> & list)
{
  ClearItemsList();
  urlMap.clear ();
  for (int i = 0; i < list.count(); ++i) {
    AradoUrl aurl (list.at(i));
    UrlDisplayViewItem * item = new UrlDisplayViewItem(aurl);
    items.append(item);
    urlMap [aurl.Hash()] = aurl;
  }
}

void
UrlDisplayWebView::UrlViewLinkClicked (const QUrl & url)
{
  QString scheme = url.scheme();
  if (scheme != "arado") {
    QDesktopServices::openUrl (url);
  } else {
    QString function = url.host();
    QString hash = url.path().mid(1);
    if (urlMap.contains (hash)) {
      AradoUrl aUrl = urlMap[hash];
      if (function == "localbrowse") {
        BrowseUrl (aUrl.Url());
      } else if (function == "externbrowse") {
        QDesktopServices::openUrl (aUrl.Url());
      } else {
        QMessageBox menuBox;
        menuBox.setText ("This is where the context menu goes");
        menuBox.setInformativeText (url.toString());
        menuBox.exec ();
      }
    } else {
      qDebug () << " BAD internal link " << url.toString();
    }
  }
}

void
UrlDisplayWebView::BrowseUrl (const QUrl & url)
{
  UrlDisplayWebViewTab * tab = new UrlDisplayWebViewTab(url, ui->browserTabWidget);
  QObject::connect(tab, SIGNAL(PageNameChanged(UrlDisplayWebViewTab*,QString)),
                   this, SLOT(TabNameChanged(UrlDisplayWebViewTab*,QString)));
  QObject::connect(tab, SIGNAL(CloseTab(UrlDisplayWebViewTab*)), this, SLOT(CloseTab(UrlDisplayWebViewTab*)));

  ui->browserTabWidget->addTab(tab, url.toString());
  ui->browserTabWidget->setCurrentWidget(tab);
}

void
UrlDisplayWebView::TabNameChanged(UrlDisplayWebViewTab * tab, const QString & name)
{
  if (tab) {
    ui->browserTabWidget->setTabText(ui->browserTabWidget->indexOf(tab), name);
  }
}

void
UrlDisplayWebView::CloseTab(UrlDisplayWebViewTab * tab)
{
  if (tab) {
    ui->browserTabWidget->removeTab(ui->browserTabWidget->indexOf(tab));
    delete tab;
  }
}

void
UrlDisplayWebView::ClearItemsList()
{
  for (int i = items.count() - 1; i >= 0; --i) {
    delete items.takeAt(i);
  }
}

}
