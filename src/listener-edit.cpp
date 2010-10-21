
#include "listener-edit.h"

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

#include "deliberate.h"
#include <QUuid>
#include <QMessageBox>
#include <QString>
#include <QDebug>

using namespace deliberate;

namespace arado 
{

ListenerEdit::ListenerEdit (QWidget *parent)
  :QDialog (parent)
{
  ui.setupUi (this);
  connect (ui.saveButton, SIGNAL (clicked()), this, SLOT (Save ()));
  connect (ui.cancelButton, SIGNAL (clicked()), this, SLOT (Cancel ()));
}

void
ListenerEdit::Run ()
{
  QString addr ("localhost");
  addr = Settings().value ("http/address",addr).toString();
  quint16 port (29998);
  port = Settings().value ("http/port",port).toUInt();
  QUuid uuid = QUuid::createUuid();
  uuid = QUuid (Settings().value ("personal/uuid",
                     uuid.toString()).toString());
  bool runServer (false);
  runServer = Settings().value ("http/run",runServer).toBool ();
  ui.addressEdit->setText (addr);
  ui.portBox->setValue (port);
  ui.uuidEdit->setText (uuid.toString());
  ui.onCheck->setChecked (runServer);
  ui.offCheck->setChecked (!runServer);
  show ();
}

void
ListenerEdit::Cancel ()
{
  reject ();
}

void
ListenerEdit::Save ()
{
  QString addr = ui.addressEdit->text ();
  quint16 port = ui.portBox->value ();
  QUuid uuid = QUuid (ui.uuidEdit->text());
  bool  runServer = ui.onCheck->isChecked();
  bool save (false);
  if (uuid.isNull()) {
    QMessageBox  reallyBox (this);
    reallyBox.setWindowTitle(tr("Arado"));
    reallyBox.setIconPixmap(QPixmap(":/images/messagebox_info.png"));
    reallyBox.setIcon (QMessageBox::Warning);
    reallyBox.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
    reallyBox.setText (tr("The UUID is not valid!\n"
                          "Proceed anyway?"));
    int result = reallyBox.exec ();
    if (result & QMessageBox::Yes) {
      save = true;
    }
  } else {
    save = true;
  }
  if (save) {
    Settings().setValue ("http/address",addr);
    Settings().setValue ("http/port",port);
    Settings().setValue ("personal/uuid",uuid.toString());
    Settings().setValue ("http/run",runServer);
    Settings().sync();
    accept ();
    emit SuggestRestart ();
  }
}


} // namespace
