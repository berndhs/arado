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
  /** Elements:
    *   %1 title-link
    *   %2 title text
    *   %3 hash-link
    *   %4 hash text
    *   %5 url-local-browse-link
    *   %6 url-external-browse-link
    *   %7 url text
    *   %8 timestamp
    *   %9 mail-link
    *   %10 copy-link
    *   %11 crawl-link
    *   %12 keyword-list-link
    *   %13 keyword-list text
    
   */
  QString homeLang("en");
  homeLang = Settings().value ("personal/language",homeLang).toString();
  QString html_str;
  html_str = "<div class=\"item\">\n"

             "<div class=\"thumb\"><a class=\"thumb\" href=\"%5\">"
                    "<img src=\"qrc:/html/html/images/arado-logo-colo-32.png\">"
                    "</a></div>"

             "<div class=\"title\">"
             "<a class=\"title\" href=\"%1\">%2</a></div>";
  bool haveKeywords = url.Keywords().count() > 0;
  QString keywordPart;
  if (haveKeywords) {
    keywordPart = QString ("<a href=\"%12\" span class=\"keywords\">")
                  + QObject::tr ("Keywords:")
                  + QString (" %13</a></span><br>");
  }
  html_str.append (keywordPart);
  html_str.append ("<a href=\"%3\" class=\"flash\">");
  html_str.append(QObject::tr("Arado-Flashmark: "));
  html_str.append (
             "<span style=\"font-size:small;"
             " font-weight:normal\">%4 &nbsp; &bull; &nbsp;</span></a>");
  html_str.append ("<span class=\"timestamp\">%8");
  html_str.append ("&nbsp; &bull; &nbsp;"
                   "[ <a href=\"http://translate.google.com/translate?hl=");
  html_str.append (homeLang);
  html_str.append ("&sl=auto&u=%7\">");
  html_str.append (QString ("%1</a>&nbsp;] &nbsp; </span>")
                     .arg(QObject::tr("Translation")));
  html_str.append (
             "<span class=\"kugar\"><a class=\"kugar\" href=\"%6\">"
             "<img src=\"qrc:/html/html/images/kugar.png\"></a>"
             "&nbsp; <a class=\"mailbutton\" href=\"%9\">"
              "<img src=\"qrc:/html/html/images/mail.png\"></a>"
             "&nbsp; <a class=\"copybutton\" href=\"%10\">"
              "<img src=\"qrc:/html/html/images/copy.png\"></a>"
             //
             // Function to crawl in deamon process what links to this url
             // using this lookup:
             // e.g. http://blekko.com/ws/http:%2F%2Fdooble.sf.net%2F+/links
             // as code:
             // "http://blekko.com/ws/
             // "%1" (THE URL)- e.g.: html_str.append (QString ("%1").arg);
             // "%2F+/links"
             "&nbsp; <a class=\"crawlbutton\" href=\"%11\""
              "<img src=\"qrc:/html/html/images/openmielke.png\"></a>"
             // Tooltip: "Aradofy what links to this URL."
             //
             "</span>\n "
             "<div class=\"url\">");
  html_str.append ("%7\n"
                   "</div>"
                  "</div>\n");
  if (haveKeywords) {
    return html_str.arg (FlashLink ("description")) .arg (url.Description())
                 .arg (FlashLink ("hash")) . arg (Flashmark())
                 .arg (FlashLink ("localbrowse"))
                 .arg (FlashLink ("externbrowse"))
                 .arg (url.Url().toString())
                 .arg (Timestamp ())
                 .arg (FlashLink ("mail"))
                 .arg (FlashLink ("copy"))
                 .arg (FlashLink ("crawl"))
                 .arg (FlashLink ("keywords"))
                 .arg (Keywords());
  } else {
    return html_str.arg (FlashLink ("description")) .arg (url.Description())
                 .arg (FlashLink ("hash")) . arg (Flashmark())
                 .arg (FlashLink ("localbrowse"))
                 .arg (FlashLink ("externbrowse"))
                 .arg (url.Url().toString())
                 .arg (Timestamp ())
                 .arg (FlashLink ("mail"))
                 .arg (FlashLink ("copy"))
                 .arg (FlashLink ("crawl"));
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

QString
UrlDisplayViewItem::FlashLink (const QString & type)
{
  QUrl flashUrl;
  flashUrl.setScheme ("arado");
  flashUrl.setHost (type);
  flashUrl.setPath (url.Hash());
  return flashUrl.toString();
}

}
