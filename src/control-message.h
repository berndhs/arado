#ifndef CONTROL_MESSAGE_H
#define CONTROL_MESSAGE_H

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

#include <QString>
#include <map>

namespace arado
{

/** Arado Control Messages 
  */

class ControlMessage
{ 
public:

  ControlMessage ();
  ControlMessage (const ControlMessage & other);
  ControlMessage & operator = (const ControlMessage & other);

  ControlMessage (const QString & cmd);

  QString Cmd () const;
  QString Value (const QString & key) const;
  void    SetCmd (const QString & cmd);
  void    SetValue (const QString & key, const QString & value);

  const std::map <QString, QString> & ValueMap () const;
 
private:

  QString  command;
  std::map  <QString, QString>  values;

};

} // namespace

#endif
