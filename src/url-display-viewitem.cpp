#include "url-display-viewitem.h"

#include <QDateTime>

namespace arado
{

UrlDisplayViewItem::UrlDisplayViewItem(const arado::AradoUrl url)
{
  this->url = url;
}

QString
UrlDisplayViewItem::Html()
{
  QString html_str;
  html_str = "<div class=\"item\">\n"
             "<div class=\"kugar\"><a href=\"%1\">"
                  "<img src=\"qrc:/html/html/images/kugar.png\">"
                  "</a></div>"
             "<h4>"
             "<span style=\"font-size:smaller; font-weight:normal\">%3 &nbsp;</span>"
             "<a href=\"%1\">%2</a></h4>\n"
             "<small>";
  if (url.Keywords().count() > 0) {
    html_str.append(QObject::tr("<span style=\"font-style:italic\">"
                                "Keywords: %4 </span>"));
  } else {
    html_str.append(" %4 \n");   
  }
  html_str.append(QObject::tr("Arado-Flashmark:"));
  html_str.append(" %5<br>\n"
                  "%1</small>\n"
                  "</div>\n");

  return html_str.arg(url.Url().toString())
                 .arg(url.Description())
                 .arg(Timestamp())
                 .arg(Keywords())
                 .arg(Flashmark());
}

QString
UrlDisplayViewItem::Timestamp()
{
  return QDateTime::fromTime_t(url.Timestamp()).toString();
}

QString
UrlDisplayViewItem::Keywords()
{
  return url.Keywords().join("; ");
}

QString
UrlDisplayViewItem::Flashmark()
{
  return QString(url.Hash().toUpper());
}

}
