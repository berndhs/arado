#include "url-display-webview.h"
#include "ui_url-display-webview.h"

#include "url-display-viewitem.h"
#include "url-display-webviewtab.h"

#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDesktopServices>

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
  for (int i = 0; i < list.count(); ++i) {
    UrlDisplayViewItem * item = new UrlDisplayViewItem(list.at(i));
    items.append(item);
  }
}

void
UrlDisplayWebView::UrlViewLinkClicked(const QUrl & url)
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
