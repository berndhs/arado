#ifndef LISTENER_EDIT_H
#define LISTENER_EDIT_H

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

#include "ui_listener-edit.h"
#include <QDialog>

namespace arado
{
class ListenerEdit : public QDialog 
{
Q_OBJECT

public:

  ListenerEdit (QWidget * parent=0);

  void Run ();

private slots:

  void Save ();
  void Cancel ();
  void useproxyguicheckbox(bool show);
  void NewUuid ();

signals:

  void SuggestRestart ();

private:

  Ui_ListenerEdit    ui;
};

} // namespace

#endif
