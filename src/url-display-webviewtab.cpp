#include "url-display-webviewtab.h"
#include "ui_url-display-webviewtab.h"


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


#include <QWebFrame>
#include <QDesktopServices>

namespace arado
{

UrlDisplayWebViewTab::UrlDisplayWebViewTab(const QUrl & url, QWidget *parent) :
  QWidget(parent),
  ui(new Ui::UrlDisplayWebViewTab)
{
  ui->setupUi(this);

  QObject::connect(ui->webView->page(), SIGNAL(loadFinished(bool)),
                   this, SLOT(FinishedLoading(bool)));
  QObject::connect(ui->closeButton, SIGNAL(clicked()),
                   this, SLOT(ClosePage()));
  QObject::connect(ui->webView, SIGNAL(urlChanged(const QUrl &)),
                   this, SLOT(UrlChanged(const QUrl &)));
  QObject::connect(ui->openInBrowserButton, SIGNAL(clicked()),
                   this, SLOT(OpenInBrowser()));
  QObject::connect(ui->webView->page(), SIGNAL(loadStarted()),
                   this, SLOT(StartedLoading()));
  QObject::connect(ui->backButton, SIGNAL(clicked()),
                   ui->webView, SLOT(back()));
  QObject::connect(ui->forwardButton, SIGNAL(clicked()),
                   ui->webView, SLOT(forward()));
  QObject::connect(ui->webView->page(), SIGNAL(loadProgress(int)), ui->progressBar, SLOT(setValue(int)));

  ui->webView->setUrl(url);
}

UrlDisplayWebViewTab::~UrlDisplayWebViewTab()
{
  delete ui;
}

void
UrlDisplayWebViewTab::FinishedLoading(bool)
{
  ui->progressBar->hide();
  emit PageNameChanged(this, ui->webView->page()->mainFrame()->title());
}

void
UrlDisplayWebViewTab::StartedLoading()
{
  ui->progressBar->show();
}

void
UrlDisplayWebViewTab::UrlChanged(const QUrl & url)
{
  ui->urlLineEdit->setText(url.toString());
}

void
UrlDisplayWebViewTab::ClosePage()
{
  ui->urlLineEdit->setText("aaaaa");
  emit CloseTab(this);
}

void
UrlDisplayWebViewTab::OpenInBrowser()
{
  QDesktopServices::openUrl(ui->webView->url());
}

}
