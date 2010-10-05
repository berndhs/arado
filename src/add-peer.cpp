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

namespace arado 
{

AddPeerDialog::AddPeerDialog (QWidget *parent)
  :QDialog (parent)
{
  addrUi.setupUi (this);
  hide ();
  connect (addrUi.okButton, SIGNAL (clicked()), this, SLOT (Ok()));
  connect (addrUi.cancelButton, SIGNAL (clicked()), this, SLOT (Cancel()));
  addrUi.ipv4Button->setChecked (true);
  addressType [addrUi.ipv4Button] = QString ("4");
  addressType [addrUi.ipv6Button] = QString ("6");
  addressType [addrUi.dnsButton]  = QString ("name");
  addressType [addrUi.i2pButton]  = QString ("i2p");
  addrUi.remoteButton->setChecked (true);
  levelType [addrUi.constantButton] = QString ("A");
  levelType [addrUi.remoteButton] = QString ("B");
}

void
AddPeerDialog::Run ()
{
  addrUi.nickEdit->setText (tr("New Nick"));
  addrUi.addressEdit->clear ();
  addrUi.portEdit->clear ();
  show ();
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
  emit NewPeer (addrUi.addressEdit->text(),
                addrUi.nickEdit->text(),
                kind, level, port);
  accept ();
}

void
AddPeerDialog::Cancel ()
{
  reject ();
}

} // namespace

