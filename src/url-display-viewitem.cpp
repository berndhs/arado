#include "url-display-viewitem.h"


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



#include "deliberate.h"
#include <QDateTime>
#include <QDebug>


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

             "<div class=\"thumb\"><a href=\"%1\">"
                    "<img src=\"qrc:/html/html/images/arado-logo-colo-32.png\">"
                    "</a></div>"

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
             " font-weight:normal\">%3 &nbsp; &bull; &nbsp;</span>");

  /*QString aicon (QString("<img src=\"qrc:/arado-logo-colo-128.png\""
                   " height=%1px weight=%2px >")
                   .arg (iconHeight).arg(iconWidth));
  html_str.append (aicon); */

  html_str.append(" %4\n");
  html_str.append ("&nbsp; &bull; &nbsp; "
    "[ <a href=\"http://translate.google.com/translate?hl=de&sl=en&u=%1\">");
  html_str.append (QString ("%1</a> ]\n"
                   "<div class=\"url\">").arg("Translation"));
  html_str.append ("%1</small>\n"
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
