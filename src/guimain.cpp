#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QTextCodec>
#include <QDebug>
#include <QDir>
#include "aradogui.h"
#include "version.h"
#include "deliberate.h"
#include "delib-debug.h"
#include "cmdoptions.h"


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

int
main (int argc, char *argv[])
{
  QCoreApplication::setApplicationName ("arado");
  QCoreApplication::setOrganizationName ("Arado");
  QCoreApplication::setOrganizationDomain ("arado.sourceforge.net");
  deliberate::ProgramVersion pv ("Arado");
  QCoreApplication::setApplicationVersion (pv.VersionNumber());
  deliberate::DSettings  settings;
  deliberate::SetSettings (settings);
  settings.setValue ("program",pv.MyName());

  QApplication app (argc, argv);

#ifndef Q_OS_DARWIN
  // Gnome on Ubuntu has menu icons disabled by default.  I think that's a bad
  // idea, and makes some menus in Clementine look confusing.
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, false);
#else
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
#endif

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
  QString logfile ("/dev/null");
  if (logDebug) {
    opts.SetStringOpt ("logdebug",logfile);
    deliberate::StartFileLog (logfile);
  }


  QString shortLang (locale.left(2));
  if (opts.SeenOpt ("lang")) {
    QString newlocale (locale);
    opts.SetStringOpt ("lang",newlocale);
    if (newlocale != locale) {   
      QString xlateFile (QString ("arado_") + newlocale);
      QString langDir (":/translate");
      translate.load (xlateFile, langDir);
      QTextCodec::setCodecForTr (QTextCodec::codecForName ("utf8"));
      app.installTranslator (&translate);
      shortLang = newlocale.left(2);
    }
  }
  deliberate::Settings().setValue("databases/spoton_shared_db",
				  QDir::homePath() + QDir::separator() +
 				  ".spot-on" + QDir::separator() +
				  "shared.db");
  shortLang = deliberate::Settings().value ("personal/language",shortLang)
                         .toString();
  deliberate::Settings().setValue ("personal/language",shortLang);
  /** the real main program starts here **/
  int appresult (0);
  bool again (false);
  do {
    arado::AradoMain  arado (0,&app);

    arado.setWindowIcon (QIcon (":/arado-logo-colo-128.png"));
    app.setWindowIcon (arado.windowIcon());
    arado.DebugOpts (showDebug, logDebug, logfile);
    arado.Start ();

    appresult = app.exec ();
    again = arado.Again ();
  } while (again);

  return appresult;


}
