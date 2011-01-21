#ifndef URLDISPLAYWIDGET_H
#define URLDISPLAYWIDGET_H

#include <QWidget>
#include <QList>

namespace arado
{

class AradoUrl;
class UrlDisplayViewItem;

class UrlDisplayView : public QWidget
{
  Q_OBJECT

public:
  UrlDisplayView(QWidget * parent=0);

  virtual void ShowUrls(QList<arado::AradoUrl> & urls) = 0;
  virtual void Updating() = 0;
  virtual void UpdatingFinished() = 0;
  virtual void ClearContents() = 0;
  virtual void ClearSelection() = 0;
  virtual void SortByTime(Qt::SortOrder) = 0;

  static bool GetConvertedYoutubeUrl(QString &);
};

}

#endif // URLDISPLAYWIDGET_H
