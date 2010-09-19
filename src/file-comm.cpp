
/****************************************************************
 * This fileBuf is distributed under the following license:
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

void
LookAhead (QFile & fileBuf, int count=1)
{
  QByteArray data;
  char byte;
  for (int i=0;i<count;i++) {
    fileBuf.getChar (&byte);
    data.append (byte);
  }
  qDebug () << " look ahead " << count << ": [" << data << "]";
  for (int j=count-1; j>=0; j--) {
    fileBuf.ungetChar (data[j]);
  }
}

namespace arado
{

FileComm::FileComm ()
{
}

QString
FileComm::Protocol ()
{
  return QString ("AradoFile");
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
  return fileBuf.isOpen ();
}

void
FileComm::Disconnect ()
{
}

bool
FileComm::IsEnd ()
{
  return fileBuf.atEnd();
}

bool
FileComm::Open (QString filename, QIODevice::OpenMode mode)
{
  fileBuf.setFileName (filename);
  bool  open = fileBuf.open (mode);
  if (fileBuf.isWritable()) {
    xmlout.setDevice (&fileBuf);
    xmlout.setAutoFormatting (true);
    xmlout.setAutoFormattingIndent (1);
  } else if (fileBuf.isReadable()) {
    parser.SetDevice (&fileBuf);
    restartHere = fileBuf.pos ();
  } else {
    return false;
  }
  return open;
}

void
FileComm::Close ()
{
  fileBuf.close ();
}

void
FileComm::Write (const AradoUrl & url)
{
  if (fileBuf.isWritable ()) {
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
qDebug () << " +++ Before Read pos " << fileBuf.pos();
  AradoUrl url;
  if (fileBuf.isReadable()) {
    fileBuf.SkipWhite ();
    parser.SetDevice (&fileBuf);
    url = parser.ReadAradoUrl ();
  }
  return url;
}

} // namespace


