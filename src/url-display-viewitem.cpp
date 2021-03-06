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
#include "url-display-view.h"
#include <QDateTime>
#include <QDebug>


using namespace deliberate;

namespace arado
{

UrlDisplayViewItem::UrlDisplayViewItem(const arado::AradoUrl url)
{
  this->url = url;
}

QString
UrlDisplayViewItem::Html()
{
  /** Elements:
    *   %1 url-local-browse-link
    *   %2 title text
    *   %3 hash-link
    *   %4 hash text
    *   %5 empty
    *   %6 url-external-browse-link
    *   %7 url text
    *   %8 timestamp
    *   %9 mail-link
    *   %10 copy-link
    *   %11 url-text-link
    *   %12 keyword-list-link
    *   %13 keyword-list text

   */
  QString html_str;
  html_str = "<div class=\"item\">\n"

             "<div class=\"thumb\"><a class=\"thumb\" href=\"%6\">"
             "<img src=\"qrc:/html/html/images/arado-logo-colo-32.png\">"
             "</a></div>"

             "<div class=\"title\">"
             "<a class=\"title\" href=\"%1\">%2</a></div>";
  bool haveKeywords = (url.Keywords().count() > 0);
  QString keywordPart;
  if (haveKeywords) {
    keywordPart = QString ("<a href=\"%12\" span class=\"keywords\">")
                  + QObject::tr ("Keywords:")
                  + QString (" %13</a></span><br>");
  } else {
    keywordPart = QString ("");
  }
  html_str.append (keywordPart);
  html_str.append ("<a href=\"%3\" class=\"flash\">");
  html_str.append(QObject::tr("Arado-Flashmark: "));
  html_str.append (
    "<span style=\"font-size:small;"
    " font-weight:normal\">%4 </span></a>");
  html_str.append ("%5");
  html_str.append ("<span class=\"timestamp\">"
                   "&nbsp; &bull; &nbsp;" 
                    "Time %8"
                   "&nbsp; &bull; &nbsp;"
                    "</span>");
  html_str.append (QString("<span class=\"translate\" >&nbsp;["
                     "<a class=\"translate\" href=\"") 
                  + FlashLink ("translate")
                  + "\">");
  html_str.append (QObject::tr("Translation")
                   +QString ("</a>] &nbsp;</span>"));
  html_str.append (
    "<span class=\"kugar\">"
    "&nbsp; <a class=\"mailbutton\" href=\"%9\">"
    "<img src=\"qrc:/html/html/images/mail.png\"></a>"
    "&nbsp; <a class=\"copybutton\" href=\"%10\">"
    "<img src=\"qrc:/html/html/images/copy.png\"></a>"
    );
  html_str.append ("&nbsp;"
    "<a class=\"pluginbutton\" href=\""
    + FlashLink ("plugin")
    + "\"><img src=\"qrc:/html/html/images/plugin.png\"></a>");
  html_str.append(
    "</span>\n "
    "<div class=\"url\">");
  html_str.append ("<a class=\"url\" href=\"%11\">%7</a>\n"
                   "</div>"
                   "</div>\n");
  if (haveKeywords) {
    return html_str.arg (FlashLink ("localbrowse"))
           .arg (url.Description())
           .arg (FlashLink ("hash"))
           .arg (Flashmark())
           .arg (QString(""))
           .arg (FlashLink ("externbrowse"))
           .arg (url.Url().toString())
           .arg (Timestamp ())
           .arg (FlashLink ("mail"))
           .arg (FlashLink ("copy"))
           .arg (FlashLink ("urltext"))
           .arg (FlashLink ("keywords"))
           .arg (Keywords());
  } else {
    return html_str.arg (FlashLink ("localbrowse"))
           .arg (url.Description())
           .arg (FlashLink ("hash"))
           .arg (Flashmark())
           .arg (QString(""))
           .arg (FlashLink ("externbrowse"))
           .arg (url.Url().toString())
           .arg (Timestamp ())
           .arg (FlashLink ("mail"))
           .arg (FlashLink ("copy"))
           .arg (FlashLink ("urltext"));
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
  return url.Keywords().join(QObject::tr("; "));
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
  flashUrl.setScheme ("aradolink");
  flashUrl.setHost (type);
  flashUrl.setPath (url.Hash());
  return flashUrl.toString();
}


}
