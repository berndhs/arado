
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
AradoStreamParser::SetInDevice (QIODevice * dev, bool clear)
{
  if (clear) {
    xmlin.clear ();
  }
  xmlin.setDevice (dev);
}

void
AradoStreamParser::SetOutDevice (QIODevice * dev)
{
  xmlout.setDevice (dev);
  xmlout.setAutoFormatting (true);
  xmlout.setAutoFormattingIndent (1);
}


AradoUrlList
AradoStreamParser::ReadAradoUrlList ()
{
  qDebug () << "ASP " << " ---vv---- Start Reading ----vv---- ";
  AradoUrl url;
  AradoUrlList  list;
  QXmlStreamReader::TokenType tok;
  tok = SkipWhite (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << tok << Info (xmlin) ;
  if (tok == QXmlStreamReader::StartDocument) {
    ParseAradoMsg (list, xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << tok << Info (xmlin) ;
  }
  qDebug () << "ASP " << " ---^^---- Done Reading ----^^---- ";
  return list;
}

bool
AradoStreamParser::ParseAradoMsg (AradoUrlList & list, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  bool ok (false);
  if (tok == QXmlStreamReader::StartElement) {
    QString tag = xmlin.name ().toString();
    if (tag.toLower() == "arado") {
      do {
        AradoUrl  url;
        tok = SkipWhite (xmlin);  
        if (tok == QXmlStreamReader::StartElement
            && xmlin.name() == QString("aradourl")) {
          ok = ParseAradoUrl (url, xmlin);
        }
        if (xmlin.tokenType() == QXmlStreamReader::EndElement
            && xmlin.name() == QString ("arado")) {
          break;
        }
        if (url.IsValid ()) {
          list.append (url);
        }
      } while (ok);
    }
  }
  SkipWhite (xmlin);
  qDebug () << "ASP " << __LINE__ << " ok " << ok << " token " << tok << Info (xmlin) ;
  return ok;
}

bool
AradoStreamParser::ParseAradoUrl (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << "ASP " << " Parse AradoUrl";
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  bool ok (false);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
qDebug () << "ASP " << " start element kind " << kind;
    if (kind == QString ("keyword")) {
      ok &= ParseKeywordElt (url, xmlin);
    } else if (kind == QString ("url")) {
      ok |= ParseUrlElt (url, xmlin);
    } else if (kind == QString ("description")) {
      ok &= ParseDescElt (url, xmlin);
    } else if (kind == QString ("hash")) {
      ok &= ParseHashElt (url, xmlin);
    } else {
      qDebug () << "ASP " << " error at " << kind << " text " << xmlin.text();
    }
    qDebug () << "ASP " << " after parse element " << Info (xmlin);
    tok = SkipEnd (xmlin);
    qDebug () << "ASP " << " bottom of element loop " << Info (xmlin);
  }
  qDebug () << "ASP " << "ok " << ok << " dropped out with " << tok << " " << Info (xmlin);
  return ok && url.IsValid ();
}

bool
AradoStreamParser::ParseKeywordElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << "ASP " << " Parse Keyword";
  ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString();
  url.AddKeyword (value);
qDebug () << "ASP " << " added keyword " << value;
  return true;
}

bool
AradoStreamParser::ParseUrlElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << "ASP " << " Parse Url ";
  ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetUrl (QUrl (value));
  qDebug () << "ASP " << " set Url " << url.Url();
  return QUrl (value).isValid ();
}

bool
AradoStreamParser::ParseDescElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << "ASP " << " Parse Description ";
  ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetDescription (value);
qDebug () << "ASP " << " added description " << value;
  return true;
}

bool
AradoStreamParser::ParseHashElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
qDebug () << "ASP " << " Parse Hash ";
  ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetHash (value.toUtf8());
qDebug () << "ASP " << " added Hash " << value;
  return true;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipWhite (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  if (tok == QXmlStreamReader::Characters
      && xmlin.isWhitespace ()) {
    tok = ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << tok << Info (xmlin);
  }
  qDebug () << "ASP " << " return " << tok;
  return tok;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipEnd (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = SkipWhite (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  if (tok == QXmlStreamReader::EndElement) {
qDebug () << "ASP " << " was end of " << xmlin.name();
    tok = SkipWhite (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << tok << Info (xmlin);
  }
  qDebug () << "ASP " << " return " << tok;
  return tok;
}

QXmlStreamReader::TokenType
AradoStreamParser::ReadToken (QXmlStreamReader & xmlin)
{  
  QXmlStreamReader::TokenType tok = xmlin.readNext ();
  qDebug () << "ASP " << __LINE__ << " read token " << tok << " " << Info (xmlin);
  qDebug () << "ASP " << " return " << tok;
  return tok;
}


void
AradoStreamParser::Write (const AradoUrl & url, bool isPartial)
{
  if (xmlout.device()->isWritable ()) {
    if (!isPartial) {
      xmlout.writeStartDocument ();
      xmlout.writeStartElement ("arado");
    }
    xmlout.writeStartElement ("aradourl");
    xmlout.writeTextElement ("hash",url.Hash());
    xmlout.writeTextElement ("url",url.Url().toString());
    QStringList kws = url.Keywords ();
    for (int k=0; k< kws.size(); k++) {
      xmlout.writeTextElement ("keyword",kws.at(k));
    }
    xmlout.writeTextElement ("description",url.Description());
    xmlout.writeEndElement (); // aradourl
    if (!isPartial) {
      xmlout.writeEndElement (); // arado
      xmlout.writeEndDocument ();
    }
  }
}

void
AradoStreamParser::Write (const AradoUrlList & list)
{
  if (xmlout.device()->isWritable ()) {
    xmlout.writeStartDocument ();
    xmlout.writeStartElement ("arado");
    for (int i=0;i<list.size();i++) {
      Write (list.at(i), true);
    }
    xmlout.writeEndElement (); // arado
    xmlout.writeEndDocument ();
  }
}

} // namespace


