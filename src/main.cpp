#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QTextCodec>
#include <QDebug>
#include "aradomain.h"
#include "version.h"
#include "deliberate.h"
#include "delib-debug.h"
#include "cmdoptions.h"


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

int
main (int argc, char *argv[])
{
  QCoreApplication::setApplicationName ("arado");
  QCoreApplication::setOrganizationName ("Arado");
  QCoreApplication::setOrganizationDomain ("arado.sourceforge.net");
  deliberate::ProgramVersion pv ("Arado");
  QSettings  settings;
  deliberate::SetSettings (settings);
  settings.setValue ("program",pv.MyName());

  QApplication app (argc, argv);

  QString locale = QLocale::system().name();
  QTranslator  translate;
  QString xlateFile (QString ("arado_") + locale);
  QString langDir (":/translate");
  translate.load (xlateFile, langDir);
  QTextCodec::setCodecForTr (QTextCodec::codecForName ("utf8"));
  app.installTranslator (&translate);
  deliberate::CmdOptions  opts ("Arado");
  opts.AddSoloOption ("debug","D",QObject::tr("show Debug log window"));
  opts.AddStringOption ("logdebug","L",QObject::tr("write Debug log to file"));
  opts.AddStringOption ("lang","l",
                   QObject::tr("language (2-letter lower case)"));

  arado::AradoMain  arado (0,&app);

  deliberate::UseMyOwnMessageHandler ();

  bool optsOk = opts.Parse (argc, argv);
  if (!optsOk) {
    opts.Usage ();
    exit(1);
  }
  if (opts.WantHelp ()) {
    opts.Usage ();
    exit (0);
  }
  pv.CLIVersion ();
  if (opts.WantVersion ()) {
    exit (0);
  }
  bool showDebug = opts.SeenOpt ("debug");

  deliberate::StartDebugLog (showDebug);
  bool logDebug = opts.SeenOpt ("logdebug");
  if (logDebug) {
    QString logfile ("/dev/null");
    opts.SetStringOpt ("logdebug",logfile);
    deliberate::StartFileLog (logfile);
  }


  if (opts.SeenOpt ("lang")) {
    QString newlocale (locale);
    opts.SetStringOpt ("lang",newlocale);
    if (newlocale != locale) {   
      QString xlateFile (QString ("arado_") + newlocale);
      QString langDir (":/translate");
      bool found = translate.load (xlateFile, langDir);
      QTextCodec::setCodecForTr (QTextCodec::codecForName ("utf8"));
      app.installTranslator (&translate);
    }
  }

  /** the real main program starts here **/

  arado.Start ();

  int appresult = app.exec ();

  return appresult + 42;


}
