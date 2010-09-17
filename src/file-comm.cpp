
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

#include "file-comm.h"
#include <QDebug>

namespace arado
{

FileComm::FileComm ()
{
}

QString
FileComm::Protocol ()
{
  return QString ("File");
}

QString
FileComm::ProtoVersion ()
{
  return QString ("0.1");
}

bool
FileComm::Connect (AradoAddress * addr)
{
  Q_UNUSED (addr)
  return false;
}

bool
FileComm::Listen ()
{
  return false;
}

bool
FileComm::IsConnected ()
{
  return file.isOpen ();
}

void
FileComm::Disconnect ()
{
}

bool
FileComm::Open (QString filename, QIODevice::OpenMode mode)
{
  file.setFileName (filename);
  bool  open = file.open (mode);
  if (file.isWritable()) {
    xmlout.setDevice (&file);
    xmlout.setAutoFormatting (true);
    xmlout.setAutoFormattingIndent (1);
  } else {
    xmlin.setDevice (&file);
  }
  return open;
}

void
FileComm::Close ()
{
  file.close ();
}

void
FileComm::Write (const AradoUrl & url)
{
  if (file.isWritable ()) {
    xmlout.writeStartDocument ();
    xmlout.writeStartElement ("arado");
    xmlout.writeStartElement ("aradourl");
    xmlout.writeAttribute ("hash",url.Hash());
    xmlout.writeTextElement ("url",url.Url().toString());
    QStringList kws = url.Keywords ();
    for (int k=0; k< kws.size(); k++) {
      xmlout.writeTextElement ("keyword",kws.at(k));
    }
    xmlout.writeTextElement ("description",url.Description());
    xmlout.writeEndElement (); // aradourl
    xmlout.writeEndElement (); // arado
    xmlout.writeEndDocument ();
  }
}

AradoUrl
FileComm::Read ()
{
  AradoUrl url;
  if (file.isReadable()) {
    QXmlStreamReader::TokenType tokt;
    tokt = xmlin.readNext ();
    if (tokt == QXmlStreamReader::StartDocument) {
      ParseAradoMsg (url, xmlin);
    }
  }
  return url;
}

void
FileComm::ParseAradoMsg (AradoUrl & url, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = xmlin.readNext ();
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
}

void
FileComm::ParseAradoUrl (AradoUrl & url, QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType  tok = SkipWhite (xmlin);
  while (tok == QXmlStreamReader::StartElement) {
    QString kind = xmlin.name().toString();
    if (kind == QString ("keyword")) {
      ParseKeywordElt (url, xmlin);
    } else if (kind == QString ("url")) {
      ParseUrlElt (url, xmlin);
    } else if (kind == QString ("description")) {
      ParseDescElt (url, xmlin);
    } else {
      qDebug () << " error at " << kind << " text " << xmlin.text();
    }
    tok = SkipEnd (xmlin);
  }
}

void
FileComm::ParseKeywordElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  xmlin.readNext();
  QString value = xmlin.text().toString();
  url.AddKeyword (value);
}

void
FileComm::ParseUrlElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  xmlin.readNext ();
  QString value = xmlin.text().toString  ();
  url.SetUrl (QUrl (value));
}

void
FileComm::ParseDescElt (AradoUrl & url, QXmlStreamReader & xmlin)
{
  xmlin.readNext ();
  QString value = xmlin.text().toString  ();
  url.SetDescription (value);
}

QXmlStreamReader::TokenType
FileComm::SkipWhite (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = xmlin.readNext();
  if (tok == QXmlStreamReader::Characters
      && xmlin.isWhitespace ()) {
    tok = xmlin.readNext();
  }
  return tok;
}

QXmlStreamReader::TokenType
FileComm::SkipEnd (QXmlStreamReader & xmlin)
{
  QXmlStreamReader::TokenType tok = SkipWhite (xmlin);
  if (tok == QXmlStreamReader::EndElement) {
    tok == SkipWhite (xmlin);
  }
}

} // namespace


