

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

#include "arado-post.h"
#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QLocale>
#include <QTextCodec>
#include <QDebug>
#include "deliberate.h"
#include "version.h"
#include "cmdoptions.h"

int
main (int argc, char * argv[])
{
  QCoreApplication::setApplicationName ("arado");
  QCoreApplication::setOrganizationName ("Arado");
  QCoreApplication::setOrganizationDomain ("arado.sourceforge.net");
  deliberate::ProgramVersion pv ("Arado");
  QCoreApplication::setApplicationVersion (pv.VersionNumber());
  deliberate::DSettings  settings;
  deliberate::SetSettings (settings);
  deliberate::Settings().SetPrefix ("post_");
  settings.setValue ("program",QString ("%1-post").arg(pv.MyName()));

  QApplication app (argc, argv, false);

  QString locale = QLocale::system().name();
  QTranslator  translate;
  QString xlateFile (QString ("arado_") + locale);
  QString langDir (":/translate");
  translate.load (xlateFile, langDir);
  QTextCodec::setCodecForTr (QTextCodec::codecForName ("utf8"));
  app.installTranslator (&translate);
  deliberate::CmdOptions  opts ("Arado-post");
  opts.AddSoloOption ("debug","D",QObject::tr("show Debug log window"));
  opts.AddStringOption ("logdebug","L",QObject::tr("write Debug log to file"));
  opts.AddStringOption ("lang","l",
                   QObject::tr("language (2-letter lower case)"));


  if (opts.SeenOpt ("lang")) {
    QString newlocale (locale);
    opts.SetStringOpt ("lang",newlocale);
    if (newlocale != locale) {   
      QString xlateFile (QString ("arado_") + newlocale);
      QString langDir (":/translate");
      translate.load (xlateFile, langDir);
      QTextCodec::setCodecForTr (QTextCodec::codecForName ("utf8"));
      app.installTranslator (&translate);
    }
  }

  /** the real main program starts here **/
  arado::AradoPost  post;
  post.SetApp (app);
  post.Start ();

  int appresult = app.exec ();
  
  return appresult;
}