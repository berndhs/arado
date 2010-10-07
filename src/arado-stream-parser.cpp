
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
  AradoUrl url;
  AradoUrlList  list;
  QXmlStreamReader::TokenType tok;
  tok = SkipWhite (xmlin);
  if (tok == QXmlStreamReader::StartDocument) {
    ParseAradoUrlMsg (list, xmlin);
  }
  return list;
}

AradoPeerList
AradoStreamParser::ReadAradoPeerList ()
{
  AradoPeer peer;
  AradoPeerList  list;
  QXmlStreamReader::TokenType tok;
  tok = SkipWhite (xmlin);
  if (tok == QXmlStreamReader::StartDocument) {
    ParseAradoPeerMsg (list, xmlin);
  }
  return list;
}

ControlMessage
AradoStreamParser::ReadControlMessage ()
{
  ControlMessage msg;
  QXmlStreamReader::TokenType tok;
  tok = SkipWhite (xmlin);
  if (tok == QXmlStreamReader::StartDocument) {
    ParseControlMsg (msg, xmlin);
  }
  return msg;
}

bool
AradoStreamParser::ParseAradoUrlMsg (AradoUrlList & list, QXmlStreamReader & xmlin)
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
      } while (ok && tok != QXmlStreamReader::Invalid);
    }
  }
  SkipWhite (xmlin);
  return ok;
}

bool
AradoStreamParser::ParseAradoPeerMsg (AradoPeerList & list, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  bool ok (false);
  if (tok == QXmlStreamReader::StartElement) {
    QString tag = xmlin.name ().toString();
    if (tag.toLower() == "arado") {
      do {
        AradoPeer  peer;
        tok = SkipWhite (xmlin);  
        if (tok == QXmlStreamReader::StartElement
            && xmlin.name() == QString("aradopeer")) {
          ok = ParseAradoPeer (peer, xmlin);
        }
        if (xmlin.tokenType() == QXmlStreamReader::EndElement
            && xmlin.name() == QString ("arado")) {
          break;
        }
        list.append (peer);
      } while (ok && tok != QXmlStreamReader::Invalid);
    }
  }
  SkipWhite (xmlin);
  return ok;
}

bool
AradoStreamParser::ParseControlMsg (ControlMessage & msg, 
                                   QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  qDebug () << "ASP " << __LINE__ << " token " << Info (xmlin);
  bool ok (false);
  if (tok == QXmlStreamReader::StartElement) {
    QString tag = xmlin.name ().toString();
    if (tag.toLower() == "arado") {
      tok = SkipWhite (xmlin);  
      if (tok == QXmlStreamReader::StartElement
          && xmlin.name() == QString("ctl")) {
        ok = ParseControlContent (msg, xmlin);
      }
    }
  }
  SkipWhite (xmlin);
  return ok;
}

bool
AradoStreamParser::ParseControlContent (ControlMessage & msg,
                                       QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  bool ok (false);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
    if (kind == QString ("cmd")) {
      ok &= ParseCmdElement (msg, xmlin);
    } else {
      ok &= ParseValueElement (msg, xmlin, kind);
    }
    tok = SkipEnd (xmlin);
  }
  return ok;
}

bool
AradoStreamParser::ParseAradoUrl (AradoUrl & url, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  bool ok (false);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
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
    tok = SkipEnd (xmlin);
  }
  return ok && url.IsValid ();
}

bool
AradoStreamParser::ParseAradoPeer (AradoPeer & peer, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  bool ok (false);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
    if (kind == QString ("address")) {
      ok &= ParsePeerAddrElt (peer, xmlin);
    } else if (kind == QString ("atype")) {
      ok |= ParsePeerATypeElt (peer, xmlin);
    } else if (kind == QString ("port")) {
      ok &= ParsePeerPortElt (peer, xmlin);
    } else if (kind == QString ("level")) {
      ok &= ParsePeerLevelElt (peer, xmlin);
    } else {
      qDebug () << "ASP " << " error at " << kind << " text " << xmlin.text();
    }
    tok = SkipEnd (xmlin);
  }
  return ok;
}

bool
AradoStreamParser::ParseKeywordElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString();
  url.AddKeyword (value);
  return true;
}

bool
AradoStreamParser::ParseUrlElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetUrl (QUrl (value));
  return QUrl (value).isValid ();
}

bool
AradoStreamParser::ParseDescElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetDescription (value);
  return true;
}

bool
AradoStreamParser::ParsePeerAddrElt (AradoPeer & peer, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  peer.SetAddr (value);
  return true;
}

bool
AradoStreamParser::ParsePeerATypeElt (AradoPeer & peer, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  peer.SetAddrType (value);
  return true;
}

bool
AradoStreamParser::ParsePeerPortElt (AradoPeer & peer, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  peer.SetPort (value.toInt());
  return true;
}

bool
AradoStreamParser::ParsePeerLevelElt (AradoPeer & peer, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  peer.SetLevel (value);
  return true;
}

bool
AradoStreamParser::ParseCmdElement (ControlMessage & msg,
                                    QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString cmd = xmlin.text().toString ();
  msg.SetCmd (cmd);
  return true;
}

bool
AradoStreamParser::ParseValueElement (ControlMessage & msg,
                                      QXmlStreamReader & xmlin,
                                      const QString & key)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString ();
  msg.SetValue (key, value);
  return true;
}


bool
AradoStreamParser::ParseHashElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  ReadToken (xmlin);
  QString value = xmlin.text().toString  ();
  url.SetHash (value.toUtf8());
  return true;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipWhite (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = ReadToken (xmlin);
  if (tok == QXmlStreamReader::Characters
      && xmlin.isWhitespace ()) {
    tok = ReadToken (xmlin);
  }
  return tok;
}

QXmlStreamReader::TokenType
AradoStreamParser::SkipEnd (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = SkipWhite (xmlin);
  if (tok == QXmlStreamReader::EndElement) {
    tok = SkipWhite (xmlin);
  }
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
AradoStreamParser::Write (const AradoPeer & peer, bool isPartial)
{
  if (xmlout.device()->isWritable()) {
    if (!isPartial) {
      xmlout.writeStartDocument ();
      xmlout.writeStartElement ("arado");
    }
    xmlout.writeStartElement ("aradopeer");
    xmlout.writeTextElement ("address", peer.Addr());
    xmlout.writeTextElement ("atype",peer.AddrType());
    xmlout.writeTextElement ("port",QString::number (peer.Port()));
    xmlout.writeTextElement ("level",peer.Level());
    xmlout.writeEndElement (); // aradopeer
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

void
AradoStreamParser::Write (const AradoPeerList & list)
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

void
AradoStreamParser::WriteUuPath (const QString & uupath,
                                const QString & status)
{
  if (xmlout.device()->isWritable ()) {
    xmlout.writeStartDocument ();
    xmlout.writeStartElement ("arado");
    xmlout.writeStartElement ("ctl");
    xmlout.writeTextElement ("cmd","uupath");
    xmlout.writeTextElement ("status",status);
    xmlout.writeTextElement ("uupath",uupath);
    xmlout.writeEndElement (); // aratoctl
    xmlout.writeEndElement (); // arado
    xmlout.writeEndDocument ();
  }
}

void
AradoStreamParser::Write (const ControlMessage & msg)
{
  if (xmlout.device()->isWritable ()) {
    xmlout.writeStartDocument ();
    xmlout.writeStartElement ("arado");
    xmlout.writeStartElement ("ctl");
    xmlout.writeTextElement ("cmd",msg.Cmd());
    std::map<QString, QString>::const_iterator it;
    const std::map<QString, QString> & values (msg.ValueMap());
    for (it = values.begin(); it != values.end(); it++) {
      xmlout.writeTextElement (it->first, it->second);
    }
    xmlout.writeEndElement ();
    xmlout.writeEndElement ();
    xmlout.writeEndDocument ();
  }
}


} // namespace


