#ifndef ARADO_STREAM_PARSER_H
#define ARADO_STREAM_PARSER_H


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

#include <QXmlStreamReader>

#include "arado-url.h"

class QIODevice;

namespace arado
{

class AradoStreamParser 
{
public:

  AradoStreamParser ();

  void              SetInDevice (QIODevice * dev, bool clear=true);
  void              SetOutDevice (QIODevice * dev);
  AradoUrlList      ReadAradoUrlList ();
  void              Write (const AradoUrl & url, bool isPartial=false);
  void              Write (const AradoUrlList & list);
  void              WriteUuPath (const QString & uupath);

private:

  QXmlStreamReader::TokenType SkipWhite (QXmlStreamReader & xmlin);
  QXmlStreamReader::TokenType SkipEnd (QXmlStreamReader & xmlin);

  QXmlStreamReader::TokenType ReadToken (QXmlStreamReader & xmlin);

  bool ParseAradoMsg (AradoUrlList & list, QXmlStreamReader & xmlin);
  bool ParseAradoUrl (AradoUrl & url, QXmlStreamReader & xmlin);
  bool ParseKeywordElt (AradoUrl & url, QXmlStreamReader & xmlin);
  bool ParseDescElt    (AradoUrl & url, QXmlStreamReader & xmlin);
  bool ParseUrlElt     (AradoUrl & url, QXmlStreamReader & xmlin);
  bool ParseHashElt    (AradoUrl & url, QXmlStreamReader & xmlin);

  QXmlStreamReader   xmlin;
  QXmlStreamWriter   xmlout;

};

} // namespace

#endif

