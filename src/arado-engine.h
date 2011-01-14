#ifndef ARADO_POST_H
#define ARADO_POST_H

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2011, Arado Team
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
#include <QApplication>
#include <QLocalServer>

#include "db-manager.h"

class QFile;
class QLocalSocket;

namespace arado
{

class Policy;
class RssPoll;

class AradoEngine : public QLocalServer 
{
Q_OBJECT

public:

  AradoEngine (QObject * parent=0);
  ~AradoEngine ();
  
  void SetApp (QApplication & qapp) { app = &qapp;}

  void Start ();

private slots:

  void Quit ();
  void StartServer ();
  void GetNewConnection ();
  void Disconnected ();
  void ReadPipe ();
  void ReportRssPoll (QString  nick);

private:

  void Bailout (const char * msg);

  QApplication   *app;
  QFile          *stdinFromMain;
  QLocalSocket   *mainPipe;

  DBManager       dbm;
  Policy         *policy;
  RssPoll        *rssPoll;

};

} // namespace

#endif
