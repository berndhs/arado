#include "version.h"
#include "deliberate.h"
#include <QMessageBox>
#include <QTimer>
#include <QCoreApplication>


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
 
namespace deliberate {

  QString ProgramVersion::MyVersionNumber ("0.1.5");

  QString ProgramVersion::MyProgramName("Arado");
  QString ProgramVersion::copyright ("Copyright (C) 2010 Arado Team");
  
  ProgramVersion::ProgramVersion (QString pgmname)
  {
    MyProgramName = pgmname;
  }
  
  QString ProgramVersion::Version ()
  { 
    return MyProgramName + QString (" Version " )
                   + MyVersionNumber + " " + QString(__DATE__) + " "
		       + QString(__TIME__)
		       + QString("\r\n")
		       + copyright;
  }

  QString ProgramVersion::VersionNumber ()
  {
    return MyVersionNumber;
  }

  void ProgramVersion::ShowVersionWindow ()
  {
    QString versionMessage(Version());
    QMessageBox box;
    box.setText (versionMessage);
    QTimer::singleShot(30000,&box,SLOT(accept()));
    box.exec();
  }
  
  void ProgramVersion::CLIVersion ()
  {
    StdOut() << Version() << endl;
  }
  
  QString ProgramVersion::MyName()
  { 
    return MyProgramName;
  }

}
