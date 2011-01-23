#include "js-view-if.h"


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


#include <QDebug>
#include <QString>
#include <QUrl>
#include <QDesktopServices>
#include <QWebFrame>
#include <QWebElement>
#include <QMenu>

namespace arado
{

JsViewInterface::JsViewInterface ()
  :language ("en")
{
}

void
JsViewInterface::openUrl (const QString & urlString)
{
  QUrl url (urlString);
  if (urlString.length() > 0 && url.isValid ()) {
    QDesktopServices::openUrl (url);
  } else {
    qDebug () << " cannot redirect to " << urlString;
  }
}

QString
JsViewInterface::getUrl (const QString & key)
{
  if (urlMap.contains (key.toUpper())) {
    return urlMap[key.toUpper()].Url().toString();
  } else {
    return QString ();
  }
}

QString
JsViewInterface::getLanguage ()
{
  return language;
}

void
JsViewInterface::setUrl (const QString & key, const AradoUrl & aUrl)
{
  urlMap[key.toUpper()] = aUrl;
}

void
JsViewInterface::setLang (const QString & lang)
{
  language = lang;
}

void
JsViewInterface::clear ()
{
  urlMap.clear ();
}

void
JsViewInterface::AddPlugin (const QString & name)
{
  if (!pluginList.contains (name)) {
    pluginList.append (name);
  }
}

void
JsViewInterface::Menu (QWebFrame * frame, const AradoUrl & aUrl)
{
  QString plugName = MenuSelectPlugin ();
  if (plugName.length() < 1) {
    return;
  }
  Call (frame, plugName, aUrl.Hash().toUpper());
}

QString
JsViewInterface::MenuSelectPlugin ()
{
  QMenu menu;
  if (pluginList.count() < 1) {
    QAction * zero = new QAction (QString("No Plugins Available"), &menu);
    zero->setData (QVariant(QString()));
    menu.addAction (zero);
  }
  for (int i=0; i<pluginList.count(); i++) {
    QAction * act = new QAction (pluginList.at(i), &menu);
    act->setData (QVariant (pluginList.at(i)));
    menu.addAction (act);
  }
  QAction * select = menu.exec (QCursor::pos());
  if (select) {
    return select->data ().toString();
  }
  return QString ();
}

void
JsViewInterface::Call (QWebFrame * frame, 
                       const QString & plugName, 
                       const QString & param)
{
  if (frame) {
    QString jscmd = QString ("Plugin%1('%2')").arg(plugName)
                                              .arg(param); 
    qDebug () << " JsViewInterface call " << jscmd;
    frame->documentElement().evaluateJavaScript (jscmd);
  }
}


} // namespace
