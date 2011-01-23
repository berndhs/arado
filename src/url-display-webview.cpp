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
#include <QWebSettings>
#include <QWebFrame>
#include <QDebug>

using namespace deliberate;

namespace arado
{

UrlDisplayWebView::UrlDisplayWebView(QWidget *parent) :
  UrlDisplayView(parent),
  ui(new Ui::UrlDisplayWebView),
  menuBusy (0)
{
  ui->setupUi(this);

  ui->urlView->page()->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
  QWebSettings * wsets = ui->urlView->page()->settings();
  wsets->setAttribute (QWebSettings::JavascriptEnabled, true);
  wsets->setAttribute (QWebSettings::JavascriptCanAccessClipboard, true);
  ui->urlView->setContextMenuPolicy(Qt::NoContextMenu);
  QObject::connect(ui->urlView, SIGNAL(linkClicked(const QUrl &)),
                   this, SLOT(UrlViewLinkClicked(const QUrl &)));
  QWebFrame * frame = ui->urlView->page()->mainFrame();
  QObject::connect(frame, SIGNAL (javaScriptWindowObjectCleared()),
                   this, SLOT (AddJsInterface ()));
  AddJsInterface ();
}

UrlDisplayWebView::~UrlDisplayWebView()
{
  ClearItemsList();
  delete ui;
}

void
UrlDisplayWebView::AddJsInterface ()
{
  ui->urlView->page()->mainFrame()->addToJavaScriptWindowObject 
                   ("AradoInterface", &jsInterface, QScriptEngine::QtOwnership);
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
  QString basic = in.readAll();
  QString headJavascript;
  LoadJavascript (headJavascript);
  basic.replace ("%%HEAD_JAVASCRIPT%%", headJavascript);
  return basic;
}

void
UrlDisplayWebView::LoadJavascript (QString & javascript)
{
  javascript.clear ();
  QString headHtml ("\n<script type=\"text/javascript\">\n");
  QString tailHtml ("\n</script>\n");
  QStringList  builtIn;
  builtIn << "Translate";
  for (int b=0; b<builtIn.count(); b++) {
    QString name = builtIn.at(b);
    QFile file (QString (":/builtinjs/%1.js").arg(name.toLower()));
    file.open (QFile::ReadOnly);
    QString jscode = file.readAll ();
    file.close ();
    javascript.append (headHtml);
    javascript.append (jscode);
    javascript.append (tailHtml);
    jsInterface.AddPlugin (name);
  }
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
  if (menuBusy > 0) {
    return;
  }
  ClearItemsList();
  urlMap.clear ();
  jsInterface.clear ();
  for (int i = 0; i < list.count(); ++i) {
    AradoUrl aurl (list.at(i));
    UrlDisplayViewItem * item = new UrlDisplayViewItem(aurl);
    items.append(item);
    urlMap [aurl.Hash()] = aurl;
    jsInterface.setUrl (aurl.Hash(), aurl);
  }
}

void
UrlDisplayWebView::UrlViewLinkClicked (const QUrl & url)
{
  QString scheme = url.scheme();
  if (scheme != "aradolink") {
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
      } else if (function == "mail") {
        menuBusy++;
        itemMenu.MenuMail (aUrl);
        menuBusy--;
      } else if (function == "keywords") {
        menuBusy++;
        itemMenu.MenuKeywords (aUrl);
        menuBusy--;
      } else if (function == "copy") {
        menuBusy++;
        itemMenu.MenuCopy (aUrl);
        menuBusy--;
      } else if (function == "crawl") {
        menuBusy++;
        itemMenu.MenuCrawl (aUrl);
        menuBusy--;
      } else if (function == "hash") {
        menuBusy++;
        itemMenu.MenuHash (aUrl);
        menuBusy--;
      } else if (function == "urltext") {
        menuBusy++;
        itemMenu.MenuUrlText (aUrl);
        menuBusy--;
      } else if (function == "plugin") {
        jsInterface.Menu (ui->urlView->page()->mainFrame(), aUrl);
      } else if (function == "translate") {
        jsInterface.Call (ui->urlView->page()->mainFrame(), "Translate", 
                             aUrl.Url());
      } else {
        menuBusy++;
        itemMenu.MenuBasic (aUrl);
        menuBusy--;
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
  QObject::connect(tab, SIGNAL(CloseTab(UrlDisplayWebViewTab*)), 
                   this, SLOT(CloseTab(UrlDisplayWebViewTab*)));

  ui->browserTabWidget->addTab(tab, url.toString());
  ui->browserTabWidget->setCurrentWidget(tab);
}

void
UrlDisplayWebView::TabNameChanged(UrlDisplayWebViewTab * tab, 
                                  const QString & name)
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
UrlDisplayWebView::ClearContents ()
{
  urlMap.clear ();
  jsInterface.clear ();
  ClearItemsList ();
}

void
UrlDisplayWebView::ClearItemsList()
{
  for (int i = items.count() - 1; i >= 0; --i) {
    delete items.takeAt(i);
  }
}

}
