#ifndef DB_MANAGER_H
#define DB_MANAGER_H

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

#include <QObject>
#include <QSqlDatabase>
#include "arado-url.h"

namespace arado
{

class DBManager : public QObject
{
Q_OBJECT

public:
  
  DBManager (QObject *parent =0);

  void  Start ();
  void  Close ();

  bool  AddUrl (AradoUrl & url);
  
  StampedUrlList  GetRecent (int howmany);

  bool  ReadKeywords (const QString & hash, QStringList & list);
  bool  ReadUrl   (const QString & hash, AradoUrl & url);
  

private:

  void StartDB  (QSqlDatabase & db, 
                 const QString & conName, 
                 const QString & dbFilename);
  void CheckFileExists (const QString & filename);
  void CheckDBComplete (QSqlDatabase & db,
                        const QStringList & elements);
  QString ElementType (QSqlDatabase & db, const QString & name);
  void    MakeElement (QSqlDatabase & db, const QString & element);
  bool    AddKeywords (AradoUrl & url);
  bool    AddUrlTimestamp (const QString & hash);

  QSqlDatabase     ipBase;
  QSqlDatabase     urlBase;

};

} // namespace

#endif
