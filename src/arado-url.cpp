
#include "arado-url.h"

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

AradoUrl::AradoUrl ()
  :valid (false),
   timestamp (0)
{
}

AradoUrl::AradoUrl (const QUrl & u)
  :valid (true),
   url (u),
   timestamp (0)
{
}

AradoUrl::AradoUrl (const AradoUrl & other)
  :valid (other.valid),
   url (other.url),
   keywords (other.keywords),
   description (other.description),
   hash (other.hash),
   timestamp (other.timestamp)
{
}

AradoUrl &
AradoUrl::operator = (const AradoUrl & other)
{
  if (this != &other) {
    valid = other.valid;
    url = other.url;
    keywords = other.keywords;
    description = other.description;
    hash = other.hash;
    timestamp = other.timestamp;
  }
  return *this;
}
 
void
AradoUrl::ComputeHash (QCryptographicHash::Algorithm  hashType)
{
  QCryptographicHash  hashData (hashType);
  hashData.addData (url.toEncoded ());
  hash = hashData.result().toHex();
}

} // namespace

