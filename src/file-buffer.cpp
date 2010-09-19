
/// FileBuffer

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

#include "file-buffer.h"
#include <string.h>
#include <QDebug>

namespace arado
{

FileBuffer::FileBuffer ()
  :QIODevice()
{}

FileBuffer::FileBuffer (QObject *parent)
  :QIODevice (parent)
  {}

FileBuffer::~FileBuffer ()
{}

void
FileBuffer::setFileName (const QString & name )
{
  file.setFileName (name);
}

QString
FileBuffer::fileName () const
{
  return file.fileName ();
}

bool
FileBuffer::open ( OpenMode mode)
{
  bool isopen;
  isopen = file.open (mode);
  if (isopen) {
    isopen = QIODevice::open (mode);
  }
  return isopen;
}

void
FileBuffer::close ()
{
  QIODevice::close ();
  file.close ();
}


void
FileBuffer::SkipWhite ()
{
  char w (' ');
  bool ok = file.getChar (&w);
  while (ok && (w == ' ' || w == '\n' || w == '\t')) {
    ok = file.getChar (&w);
  }
  if (!(w == ' ' || w == '\n' || w == '\t')) {
    file.ungetChar (w);
  }
}

qint64
FileBuffer::readData ( char * data, qint64 maxSize )
{
  static const int realMax (512);
  static qint64 total (0);
  qint64 myMax = (maxSize < realMax ? maxSize :  realMax);
  qint64 actual = file.read (data,myMax);
  total += actual;
  qDebug () << "F-B " << " readData max they " 
            << maxSize
            << " mine " << realMax
            << " actual " << actual 
            << " total " << total;
  return actual;
}

qint64
FileBuffer::writeData ( const char * data, qint64 maxSize )
{
  return file.write (data, maxSize);
}

bool
FileBuffer::atEnd () const
{
  return file.atEnd ();
}

qint64
FileBuffer::bytesAvailable () const
{
  qint64 nbytes = QIODevice::bytesAvailable ();
  return nbytes;
}

qint64
FileBuffer::bytesToWrite () const
{
  return QIODevice::bytesToWrite ();
}

bool
FileBuffer::canReadLine () const
{
  return QIODevice::canReadLine () && file.canReadLine ();
}

QString 
FileBuffer::errorString () const
{
  return file.errorString ();
}

qint64
FileBuffer::pos () const
{
  qint64 p = QIODevice::pos ();
  qDebug () << "F-B " << " pos " << p;
  return p;
}

bool
FileBuffer::reset ()
{
  bool ok = file.reset ();
  ok &= QIODevice::reset ();
  return ok;
}

bool
FileBuffer::seek ( qint64 pos )
{
  qDebug () << "F-B " << " seek to " << pos;
  return file.seek (pos);
}

qint64
FileBuffer::size () const
{
  return file.size ();
}

bool
FileBuffer::waitForBytesWritten (int msecs)
{
  return file.waitForBytesWritten (msecs);
}

bool
FileBuffer::waitForReadyRead (int msecs)
{
  return file.waitForReadyRead (msecs);
}


bool
FileBuffer::isSequential () const
{
  return true;
}


} // namespace

