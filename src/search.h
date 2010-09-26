#ifndef SEARCH_H
#define SEARCH_H

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

#include "arado-url.h"
#include "db-manager.h"
#include <QObject>
#include <QStringList>
#include <QString>

namespace arado
{
class Search : public QObject 
{
Q_OBJECT

public:

  enum Combine {
          Words_All = 1,
          Words_Any = 2,
          Words_NoneOf = 3
          } ;
  enum Result {
          Res_Table = 1,
          Res_List = 2
          } ;

  Search (QObject * parent);
  
  virtual void SetDB (DBManager * dbm);

  /** \brief These functions return an integer search Id, the signal Ready 
    * refers to this Id
    */

  virtual int ExactKeywords (const QStringList & keyList,
                           Combine combine,
                           Result resType = Res_List);
  virtual int ExactKeyword (const QString & key,
                           Combine combine = Words_All,
                           Result resType = Res_List);

  virtual bool ResultReady (int resultId);
  virtual int  ResultSize (int resultId);

  virtual bool  ResultTable (int resultId, QString & tableName);
  virtual bool  ResultList (int resultId, QStringList & hashList);

  virtual bool  Clear (int resultId);
  virtual void  ClearAll ();

signals:

  void Ready (int searchId);

private:

  DBManager  *db;

};

} // namespace

#endif
