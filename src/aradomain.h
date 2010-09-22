#ifndef ARADOMAIN_H
#define ARADOMAIN_H

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, The Arado Team
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
#include "ui_aradomain.h"
#include "db-manager.h"

#include <QMainWindow>

class QCloseEvent;
class QApplication;
class QTimer;

namespace arado
{

class ConfigEdit;
class FileComm;
class UrlDisplay;
class ConnectionDisplay;
class EntryForm;
class AradoUrl;

class AradoMain : public QMainWindow
{
Q_OBJECT

public:

  AradoMain (QWidget *parent, QApplication * pa);

  void Start ();

  void closeEvent (QCloseEvent * event);

public slots:

  void Quit ();
  void slotAbout();
  void DoConfigEdit ();
  void DoneConfigEdit (bool saved);
  void DoFileComm ();
  void DoEntry ();
  void DoneEntry ();
  void NewUrl (const AradoUrl & newurl);


private:

  void  Connect ();

  Ui_AradoWin         mainUi;
  QApplication       *app;
  bool                setupDone;
  ConfigEdit         *configEdit;
  FileComm           *fileComm;
  UrlDisplay         *urlDisplay;
  ConnectionDisplay  *connDisplay;
  EntryForm          *entryForm;
  DBManager           dbMgr;
  QTimer             *refreshUrls;

};

} // namespace

#endif
