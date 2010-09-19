
/// Arado Stream Parser

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

#include "arado-stream-parser.h"
#include <QIODevice>
#include <QString>
#include <QDebug>

namespace arado
{

QString
Info (QXmlStreamReader & xr)
{
  switch (xr.tokenType ()) {
  case QXmlStreamReader::StartDocument:
  case QXmlStreamReader::StartElement:
  case QXmlStreamReader::EndDocument:
  case QXmlStreamReader::EndElement:
    return xr.tokenString() + " " + xr.name().toString ();
    break;
  case QXmlStreamReader::Characters:
    return xr.tokenString() + " " + xr.text().toString ();
    break;
  default:
    return xr.tokenString() ;
    break;
  }
}

AradoStreamParser::AradoStreamParser ()
{
}

void
AradoStreamParser::SetDevice (QIODevice * dev)
{
  xmlin.clear ();
  xmlin.setDevice (dev);
}

AradoUrl
AradoStreamParser::ReadAradoUrl ()
{
  qDebug () << " ---vv---- Start Reading ----vv---- ";
  AradoUrl url;
  QXmlStreamReader::TokenType tok;
  tok = SkipWhite (xmlin);
  qDebug () << __LINE__ << " token " << tok << Info (xmlin) ;
  if (tok == QXmlStreamReader::StartDocument) {
    ParseAradoMsg (url, xmlin);
  qDebug () << __LINE__ << " token " << tok << Info (xmlin) ;
  }
  qDebug () << " ---^^---- Done Reading ----^^---- ";
  return url;
}

void
AradoStreamParser::ParseAradoMsg (AradoUrl & url, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  if (tok == QXmlStreamReader::StartElement) {
    QString tag = xmlin.name ().toString();
    if (tag.toLower() == "arado") {
      tok = SkipWhite (xmlin);  
      if (tok == QXmlStreamReader::StartElement
          && xmlin.name() == QString("aradourl")) {
        ParseAradoUrl (url, xmlin);
      }
    }
  }
  SkipWhite (xmlin);
  qDebug () << __LINE__ << " token " << tok << Info (xmlin) ;
}

void
AradoStreamParser::ParseAradoUrl (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << " Parse AradoUrl";
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
qDebug () << " start element kind " << kind;
    if (kind == QString ("keyword")) {
      ParseKeywordElt (url, xmlin);
    } else if (kind == QString ("url")) {
      ParseUrlElt (url, xmlin);
    } else if (kind == QString ("description")) {
      ParseDescElt (url, xmlin);
    } else {
      qDebug () << " error at " << kind << " text " << xmlin.text();
    }
    qDebug () << " after parse element " << Info (xmlin);
    tok = SkipEnd (xmlin);
    qDebug () << " bottom of element loop " << Info (xmlin);
  }
  qDebug () << " dropped out with " << tok << " " << Info (xmlin);
}

void
AradoStreamParser::ParseKeywordElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << " Parse Keyword";
  ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString();
  url.AddKeyword (value);
qDebug () << " added keyword " << value;
}

void
AradoStreamParser::ParseUrlElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << " Parse Url ";
  ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetUrl (QUrl (value));
}

void
AradoStreamParser::ParseDescElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << " Parse Description ";
  ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetDescription (value);
qDebug () << " added description " << value;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipWhite (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  if (tok == QXmlStreamReader::Characters
      && xmlin.isWhitespace ()) {
    tok = ReadToken (xmlin);
  qDebug () << __LINE__ << " token " << tok << Info (xmlin);
  }
  qDebug () << " return " << tok;
  return tok;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipEnd (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = SkipWhite (xmlin);
  qDebug () << __LINE__ << " token " << Info (xmlin);
  if (tok == QXmlStreamReader::EndElement) {
qDebug () << " was end of " << xmlin.name();
    tok = SkipWhite (xmlin);
  qDebug () << __LINE__ << " token " << tok << Info (xmlin);
  }
  qDebug () << " return " << tok;
  return tok;
}

QXmlStreamReader::TokenType
AradoStreamParser::ReadToken (QXmlStreamReader & xmlin)
{  
  QXmlStreamReader::TokenType tok = xmlin.readNext ();
  qDebug () << __LINE__ << " read token " << tok << " " << Info (xmlin);
  qDebug () << " return " << tok;
  return tok;
}


} // namespace


