/** AddPeerDialog
  */

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

#include "add-peer.h"
#include "db-manager.h"
#include "deliberate.h"
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

using namespace deliberate;

namespace arado 
{

AddPeerDialog::AddPeerDialog (QWidget *parent)
  :QDialog (parent),
   db (0),
   elvis ("Elvis")
{
  addrUi.setupUi (this);
  hide ();
  connect (addrUi.okButton, SIGNAL (clicked()), this, SLOT (Ok()));
  connect (addrUi.cancelButton, SIGNAL (clicked()), this, SLOT (Cancel()));
  connect (addrUi.addressEdit, SIGNAL (returnPressed()),
           this, SLOT (AddrReturn ()));
  addrUi.ipv4Button->setChecked (true);
  addressType [addrUi.ipv4Button] = QString ("4");
  addressType [addrUi.ipv6Button] = QString ("6");
  addressType [addrUi.dnsButton]  = QString ("name");
  addrUi.constantButton->setChecked (true);
  levelType [addrUi.constantButton] = QString ("A");
  levelType [addrUi.remoteButton] = QString ("B");
  levelType [addrUi.flexibleButton] = QString ("C");
}

void
AddPeerDialog::Run ()
{
  elvis = Settings().value ("personal/elvis",elvis).toString();
  Settings().setValue ("personal/elvis",elvis);
  addrUi.okButton->setDefault (false);
  addrUi.okButton->setAutoDefault (false);
  addrUi.cancelButton->setDefault (false);
  addrUi.cancelButton->setAutoDefault (false);
  ResetDisplay ();
  show ();
}

void
AddPeerDialog::ResetDisplay ()
{
  quint64 now = QDateTime::currentDateTime().toTime_t();
  QString defaultName (QString("%1%21").arg(elvis).arg(now));
  addrUi.nickEdit->setText (defaultName);
  addrUi.addressEdit->clear ();
  addrUi.portEdit->clear ();
  addrUi.uuidEdit->clear ();
}

void
AddPeerDialog::Ok ()
{
  QString kind("0"), level("C");
  QAbstractButton  * typeButton = addrUi.typeGroup->checkedButton ();
  QAbstractButton  * levelButton = addrUi.levelGroup->checkedButton ();
  if (addressType.contains (typeButton)) {
    kind = addressType [typeButton];
  };
  if (levelType.contains (levelButton)) {
    level = levelType [levelButton];
  }
  int port = addrUi.portEdit->text().toInt();
  QUuid uuid = QUuid (addrUi.uuidEdit->text());
  bool added = AddPeer (addrUi.nickEdit->text(),
                addrUi.addressEdit->text(),
                kind, level, port, uuid);
  if (added) {
    accept ();
  }
}

void
AddPeerDialog::Cancel ()
{
  reject ();
}

void
AddPeerDialog::AddrReturn ()
{
  QString addr = addrUi.addressEdit->text();
  if (deliberate::IsIp4Address (addr)) {
    addrUi.ipv4Button->setChecked (true);
  } else if (deliberate::IsIp6Address (addr)) {
    addrUi.ipv6Button->setChecked (true);
  } else {
    addrUi.dnsButton->setChecked (true);
  }
  addrUi.portEdit->setFocus ();
}

bool
AddPeerDialog::AddPeer (QString nick, QString addr, QString addrType,
                       QString level, int port, QUuid uuid )
{
  qDebug () << " new peer to add " << nick << addr << addrType << level << port;
  AradoPeer newPeer (nick, addr, addrType, level, port, 
                     uuid, AradoPeer::State_New);
  bool added (false);
  bool writeit (true);
  if (db) {
    bool isknown = db->HavePeer (nick);
    if (uuid.isNull()) {
      QMessageBox::StandardButton okOrNo = QMessageBox::warning (this,
           tr("Arado"), tr("The UUID is not valid!\n"
                           "Proceed anyway?"),
           (QMessageBox::Yes | QMessageBox::No));
      if (okOrNo & QMessageBox::Yes) {
        writeit = true;
      } else {
        return false;
      }
    }
    if (isknown) {
      QMessageBox box (this);
      box.setWindowTitle(tr("Arado"));
      box.setIconPixmap(QPixmap(":/images/messagebox_info.png"));
      box.setText (
               tr("Nick %1 already exists!\n"
                 "Do you want to replace it?")
               .arg (nick));
      
      box.setStandardButtons (QMessageBox::Yes 
                            | QMessageBox::No);
      int choice = box.exec ();
      writeit = (choice & QMessageBox::Yes);
      if (writeit) {
        db->RemovePeer (nick);
      }
    }
    if (writeit) {
      added = db->AddPeer (newPeer);
    }
  } else {
    qDebug () << " no DB in AddPeerDialog ";
  }
  if (added) {
    emit NewPeer (nick);
  }
  return writeit;
}


} // namespace

