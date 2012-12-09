#include "delib-rng.h"

/****************************************************************
 * This file is distributed under the following license:
 *
 * Copyright (C) 2010, Bernd Stramm
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


namespace deliberate
{

Rng_LCG_GN::Rng_LCG_GN ()
  :A(1103515245),
   C(12345),
#if defined(__LP64__) || defined(_LP64)
   M(0xffffffffffffffff),
#else
   M(0xffffffff),
#endif
   Xn(12345)
{
}

quint64
Rng_LCG_GN::Random (quint64 upper)
{
  Xn = (A * Xn + C) & M;
  return Xn % upper;
}

} // namespace

