
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
  connect (ui.uuidButton, SIGNAL (clicked()), this, SLOT (NewUuid ()));
  /* Hide Proxy gui elements */
  useproxyguicheckbox(false);
  connect(ui.useproxyguicheckbox, SIGNAL(toggled(bool)), 
                     this, SLOT(useproxyguicheckbox(bool)));
  //
  connect (ui.lineEdit_proxyhost, SIGNAL (clicked()), this, SLOT (Proxyhost ()));
  connect (ui.comboBox_proxytype, SIGNAL (clicked()), this, SLOT (Proxytype ()));
  connect (ui.lineEdit_proxyport, SIGNAL (clicked()), this, SLOT (Proxyport ()));
  connect (ui.lineEdit_proxyusername, SIGNAL (clicked()), 
                     this, SLOT (Proxyusername ()));
  connect (ui.lineEdit_proxypassword, SIGNAL (clicked()), 
                     this, SLOT (Proxypassword ()));
  //
}

void
ListenerEdit::NewUuid ()
{
  QUuid uuid = QUuid::createUuid();
  ui.uuidEdit->setText (uuid.toString());
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
  //
  QString useproxyguicheckbox ("");
  useproxyguicheckbox = Settings().value ("proxy/useproxy",useproxyguicheckbox)
                                  .toString();

  QString proxyhost ("");
  proxyhost = Settings().value ("proxy/host",proxyhost).toString();

  QString proxytype ("");
  proxytype= Settings().value ("proxy/type",proxytype).toString();

  // quint16 proxyport ("");  // -> needs a check
  // proxyport= Settings().value ("proxy/port",proxyport).toUInt();

  QString proxyusername ("");
  proxyusername= Settings().value ("proxy/user",proxyusername).toString();

  QString proxypassword ("");
  proxypassword= Settings().value ("proxy/password",proxypassword).toString();
  //
  ui.addressEdit->setText (addr);
  ui.portBox->setValue (port);
  ui.uuidEdit->setText (uuid.toString());
  ui.onCheck->setChecked (runServer);
  ui.offCheck->setChecked (!runServer);
  //
  ui.useproxyguicheckbox->setChecked(Settings().value("proxy/useproxy",false)
                                               .toBool());
  int idx=ui.comboBox_proxytype->findText(Settings().value("proxy/type","")
                                               .toString());
  if(idx>=0)  ui.comboBox_proxytype->setCurrentIndex(idx);
  ui.lineEdit_proxyhost->setText(Settings().value("proxy/host","").toString());
  ui.lineEdit_proxyport->setText(Settings().value("proxy/port","").toString());
  ui.lineEdit_proxyusername->setText(Settings().value("proxy/user","")
                                               .toString());
  ui.lineEdit_proxypassword->setText(Settings().value("proxy/password","")
                                               .toString());
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
    Settings().setValue ("proxy/useproxy",ui.useproxyguicheckbox->isChecked());
    Settings().setValue ("proxy/type",ui.comboBox_proxytype->currentText());
    Settings().setValue ("proxy/user",ui.lineEdit_proxyusername->text());
    Settings().setValue ("proxy/password",ui.lineEdit_proxypassword->text());
    Settings().setValue ("proxy/host",ui.lineEdit_proxyhost->text());
    Settings().setValue ("proxy/port",ui.lineEdit_proxyport->text());

    Settings().setValue ("http/address",addr);
    Settings().setValue ("http/port",port);
    Settings().setValue ("personal/uuid",uuid.toString());
    Settings().setValue ("http/run",runServer);
    Settings().sync();
    accept ();
    emit SuggestRestart ();
  }
}


void ListenerEdit::useproxyguicheckbox(bool useproxyguicheckbox)
{
  Q_UNUSED(useproxyguicheckbox);

    if (ui.useproxyguicheckbox->isChecked()) {
        ui.label_proxyhost->setVisible(true);
        ui.lineEdit_proxyhost->setVisible(true);
        ui.comboBox_proxytype->setVisible(true);
        ui.label_proxytype->setVisible(true);
        ui.label_proxyport->setVisible(true);
        ui.lineEdit_proxyport->setVisible(true);
        ui.label_proxyusername->setVisible(true);
        ui.lineEdit_proxyusername->setVisible(true);
        ui.label_proxypassword->setVisible(true);
        ui.lineEdit_proxypassword->setVisible(true);
        ui.portBox->setVisible(false);
        ui.label_port->setVisible(false);
        ui.addressEdit->setVisible(false);
        ui.label_address->setVisible(false);

    } else {
        ui.label_proxyhost->setVisible(false);
        ui.lineEdit_proxyhost->setVisible(false);
        ui.comboBox_proxytype->setVisible(false);
        ui.label_proxytype->setVisible(false);
        ui.label_proxyport->setVisible(false);
        ui.lineEdit_proxyport->setVisible(false);
        ui.label_proxyusername->setVisible(false);
        ui.lineEdit_proxyusername->setVisible(false);
        ui.label_proxypassword->setVisible(false);
        ui.lineEdit_proxypassword->setVisible(false);
        ui.portBox->setVisible(true);
        ui.label_port->setVisible(true);
        ui.addressEdit->setVisible(true);
        ui.label_address->setVisible(true);
    }
 }


} // namespace
