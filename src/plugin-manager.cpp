#include "plugin-manager.h"


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

#include "deliberate.h"
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QDebug>

using namespace deliberate;

namespace arado
{

PluginManager::PluginManager ()
{
}

QStringList
PluginManager::InstalledPlugins ()
{
  QString pluginPath = PluginDirPath ();
  QDir pluginDir (pluginPath);
  if (!pluginDir.exists(pluginPath)) {
    pluginDir.mkpath (pluginPath);
  }
  QStringList filters;
  filters << "*.js";
  QStringList files = pluginDir.entryList (filters,
                                      QDir::Files | QDir::Readable,
                                      QDir::Name);
qDebug () << " plugin filter " << filters;
qDebug () << " plugin file list " << files;
  for (int f=0; f<files.count(); f++) {
    files[f].chop (3);
  }
  return files;
}

bool
PluginManager::LoadJavascript (const QString & name, QString & javascript)
{
  javascript.clear ();
  QString path = PluginDirPath ();
  QString filename = path + QDir::separator() + name + ".js";
  QFile file (filename);
  bool ok = file.open (QFile::ReadOnly);
qDebug () << " open plugin file " << filename << ok;
  if (!ok) {
    return false;
  }
  javascript = file.readAll ();
  file.close ();
qDebug () << " loaded plugin " << filename;
qDebug () << " code " << javascript;
  if (javascript.contains (QString ("Plugin%1").arg(name))) {
    return true;
  } else {
    return false;
  }
}

QString
PluginManager::PluginDirPath ()
{
  QString path = QDesktopServices::storageLocation 
                   (QDesktopServices::DataLocation)
                      + QDir::separator()
                      + QString("plugins");
  path = Settings().value ("plugins/dirpath",path).toString();
  Settings().setValue ("plugins/dirpath",path);
  Settings().sync();
  return path;
}

} // namespace

