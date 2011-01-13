#ifndef URLDISPLAYWEBVIEWTAB_H
#define URLDISPLAYWEBVIEWTAB_H

#include <QWidget>

class QUrl;

namespace Ui
{
class UrlDisplayWebViewTab;
}

namespace arado
{

class UrlDisplayWebViewTab : public QWidget
{
  Q_OBJECT

public:
  explicit UrlDisplayWebViewTab(const QUrl &, QWidget *parent = 0);
  ~UrlDisplayWebViewTab();

private slots:
  void UrlChanged(const QUrl &);
  void FinishedLoading(bool);
  void StartedLoading();
  void ClosePage();
  void OpenInBrowser();

private:
  Ui::UrlDisplayWebViewTab *ui;

signals:
  void PageNameChanged(UrlDisplayWebViewTab *, const QString &);
  void CloseTab(UrlDisplayWebViewTab *);
};

}

#endif // URLDISPLAYWEBVIEWTAB_H
