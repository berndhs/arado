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
#include <QPair>

namespace arado
{

class AradoUrl
{
public:

  AradoUrl ();
  AradoUrl (const QUrl & u);
  AradoUrl (const AradoUrl & other );
  AradoUrl & operator = (const AradoUrl & other);

  QUrl Url ()                const { return url; }
  QStringList Keywords ()    const { return keywords; }
  QString     Description () const { return description; }
  QByteArray  Hash ()       const { return hash; }
  quint64     Timestamp ()  const { return timestamp; }

  bool IsValid () const { return valid; }

  void SetUrl ( const QUrl & u ) { url = u; valid = true; }
  void SetKeywords ( const QStringList & kws ) { keywords = kws; }
  void SetDescription ( const QString & desc ) { description = desc; }
  void SetHash (const QByteArray & h ) { hash = h; }
  void SetTimestamp (const quint64 & ts ) { timestamp = ts; }

  void AddKeyword ( const QString & kw ) { keywords.append (kw); }

  void ComputeHash ();


private:

  bool               valid;
  QUrl               url;
  QStringList        keywords;
  QString            description;
  QByteArray         hash;
  quint64            timestamp;

} ;

typedef QList<AradoUrl>           AradoUrlList;

} // namespace

#endif
