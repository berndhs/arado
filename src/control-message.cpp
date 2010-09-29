
#include "control-message.h"

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

namespace arado
{

ControlMessage::ControlMessage ()
{}

ControlMessage::ControlMessage (const ControlMessage & other)
  :command (other.command),
   values (other.values)
{
}

ControlMessage &
ControlMessage::operator = (const ControlMessage & other)
{
  if (this != &other) {
    command = other.command;
    values = other.values;
  }
  return *this;
}

ControlMessage::ControlMessage (const QString & cmd)
{
  command = cmd;
}

QString
ControlMessage::Cmd () const
{
  return command;
}

QString
ControlMessage::Value (const QString & key) const
{
  std::map <QString,QString>::const_iterator whereisit = values.find(key);
  if (whereisit != values.end()) {
    return whereisit->second;
  } else {
    return QString();
  }
}

void
ControlMessage::SetCmd (const QString & cmd)
{
  command = cmd;
}

void
ControlMessage::SetValue (const QString & key, const QString & value)
{
  values[key] = value;
}

const std::map <QString, QString> &
ControlMessage::ValueMap () const
{
  return values;
}
} // namespace

