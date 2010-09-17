#ifndef ARADO_URL_H
#define ARADO_URL_H

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Arado Team
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
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QByteArray>

namespace arado
{

class AradoUrl
{
public:

  AradoUrl ();
  AradoUrl (const QUrl & u):url(u) {}

  QUrl Url ()                { return url; }
  QStringList Keywords ()    { return keywords; }
  QString     Description () { return description; }
  QByteArray  Hash ()        { return hash; }

  void SetUrl ( const QUrl & u ) { url = u; }
  void SetKeywords ( const QStringList & kws ) { keywords = kws; }
  void SetDescription ( const QString & desc ) { description = desc; }

  void AddKeyword ( const QString & kw ) { keywords.append (kw); }

  void ComputeHash ();


private:

  QUrl               url;
  QStringList        keywords;
  QString            description;
  QByteArray         hash;

} ;

} // namespace

#endif
