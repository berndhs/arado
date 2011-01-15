#include "url-display-viewitem.h"
#include "deliberate.h"
#include <QDateTime>

using namespace deliberate;

namespace arado
{

UrlDisplayViewItem::UrlDisplayViewItem(const arado::AradoUrl url)
  :iconHeight (16),
   iconWidth (16)
{
  this->url = url;
  iconHeight = Settings().value ("urldisplay/aiconheight",iconHeight).toInt();
  iconWidth = Settings().value ("urldisplay/aiconwidth",iconWidth).toInt();
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
             "<a href=\"%1\">%2</a></h4>"

             "<small>";
  bool haveKeywords = url.Keywords().count() > 0;
  if (haveKeywords) {
    html_str.append(QObject::tr("<span style=\"font-style:italic\">"
                                "Keywords: %5 </span><br>"));
  }
  html_str.append(QObject::tr("Arado-Flashmark: "));
  html_str.append (
             "<span style=\"font-size:small;"
             " font-weight:normal\">%3 &nbsp;</span>");

  QString aicon (QString("<img src=\"qrc:/arado-logo-colo-128.png\""
                   " height=%1px weight=%2px >")
                   .arg (iconHeight).arg(iconWidth));
  html_str.append (aicon);

  html_str.append(" %4\n"
                  "[ <a href=\"http://translate.google.com/translate?hl=de&sl=en&u=%1\">Translate this page</a> ]\n"
                  "<br>"

                   "<div class=\"url\">"
                  "%1</small>\n"
                   "</div>"

                  "</div>\n");
  if (haveKeywords) {
    return html_str.arg(url.Url().toString())
                 .arg(url.Description())
                 .arg(Flashmark())
                 .arg(Timestamp())
                 .arg(Keywords());
  } else {
    return html_str.arg(url.Url().toString())
                 .arg(url.Description())
                 .arg(Flashmark())
                 .arg(Timestamp());
  }
}

QString
UrlDisplayViewItem::Timestamp()
{
  return QDateTime::fromTime_t(url.Timestamp()).toString("yyyy-MM-dd");
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
