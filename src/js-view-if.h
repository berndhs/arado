#ifndef ARADO_JS_VIEW_INTERFACE_H
#define ARADO_JS_VIEW_INTERFACE_H

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


#include <QObject>
#include <QString>
#include <QMap>
#include "arado-url.h"

class QWebFrame;
class QUrl;

namespace arado 
{

class JsViewInterface : public QObject
{
Q_OBJECT
public:

  JsViewInterface ();

  /** entry points available to javascript
    */

  Q_INVOKABLE   QString  getUrl  (const QString & flashmark);
  Q_INVOKABLE   QString  getLanguage ();
  Q_INVOKABLE   void     openUrl (const QString & urlString);

  /** entry points not available to javascript.
    * There is a reason for this, so DO NOT make these invokable.
    * Yes that means YOU.
    */

  void clear ();
  void setUrl  (const QString & key, const AradoUrl & aUrl);
  void setLang (const QString & lang);

  void Menu (QWebFrame * frame, const AradoUrl & aUrl);
  void Call (QWebFrame * frame, const QString & plugName, const QUrl & url);
  void Call (QWebFrame * frame, const QString & plugName, 
                                const QString & param);
  void AddPlugin (const QString & name);

private:

  QString  MenuSelectPlugin ();

  QString     language;
  QStringList pluginList;
  QMap <QString, AradoUrl>   urlMap;
};

} // namespace

#endif
