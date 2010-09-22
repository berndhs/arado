#ifndef ENTRY_FORM_H
#define ENTRY_FORM_H

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

#include "ui_entry-form.h"
#include <QWidget>
#include "arado-url.h"

namespace arado
{

class DBManager;

class EntryForm : public QWidget
{
Q_OBJECT

public:

  EntryForm (QWidget *parent);

  void SetDB (DBManager *dbm) { db = dbm; }
 
  void Start ();

private slots:

  void Done ();
  void Clear ();
  void Save ();

signals:

  void Finished ();
  void NewUrl (const AradoUrl & url);

private:

  Ui_EntryForm      ui;
  DBManager        *db;

};

} // namespace

#endif
