#ifndef HTTP_TYPES_H
#define HTTP_TYPES_H

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

enum HttpRequestType {
     HRT_None = 0,
     HRT_Request  = 1,
     HRT_Offer  = 2,
     HRT_Put = 3
    };

enum HttpDataType {
    HDT_None = 0,
    HDT_Url  = 1,
    HDT_Addr = 2
   };

} // namespace

#endif

