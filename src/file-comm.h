#ifndef FILE_COMM_H
#define FILE_COMM_H

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

#include "comm-base.h"
#include "arado-stream-parser.h"
#include "file-buffer.h"
#include <QFile>
#include <QXmlStreamWriter>
#include <QList>


namespace arado
{

class FileComm : public CommBase 
{
public:

  FileComm ();

  QString   Protocol ();
  QString   ProtoVersion ();
  bool      Connect (AradoAddress * addr);
  bool      Listen ();
  bool      IsConnected ();
  void      Disconnect ();
  AradoUrlList  Read ();
  void      Write (const AradoUrl & url, bool isPartial=false);
  void      Write (const AradoUrlList & list);
  bool      IsEnd ();

  bool      Open (QString filename, QIODevice::OpenMode mode);
  void      Close ();

private:


  FileBuffer        fileBuf;
  AradoStreamParser parser;
  int               restartHere;

};

} // namespace


#endif
