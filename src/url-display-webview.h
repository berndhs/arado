#ifndef URLDISPLAYWEBWIDGET_H
#define URLDISPLAYWEBWIDGET_H

#include "url-display-view.h"

class QUrl;

namespace Ui {
    class UrlDisplayWebView;
}

namespace arado {

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
    void ClearContents() { ClearItemsList(); }
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

    void Load(const QList<arado::AradoUrl> &);

    QList<UrlDisplayViewItem *> items;
    Ui::UrlDisplayWebView *ui;
};

}

#endif // URLDISPLAYWEBWIDGET_H
