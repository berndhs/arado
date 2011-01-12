

#/****************************************************************
# * This file is distributed under the following license:
# *
# * Copyright (C) 2010, Arado Team
# *
# *  This program is free software; you can redistribute it and/or
# *  modify it under the terms of the GNU General Public License
# *  as published by the Free Software Foundation; either version 2
# *  of the License, or (at your option) any later version.
# *
# *  This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# *  GNU General Public License for more details.
# *
# *  You should have received a copy of the GNU General Public License
# *  along with this program; if not, write to the Free Software
# *  Foundation, Inc., 51 Franklin Street, Fifth Floor, 
# *  Boston, MA  02110-1301, USA.
# ****************************************************************/
#


TEMPLATE	= app
LANGUAGE	= C++
CONFIG		+= qt warn_on build_all
QT		+= network sql xml

!include ("options.pri") {
  message ("using defaults, no options.pri")
} else {
  message ("configured for $$DISTRO")
  fedora {
  }
  ubuntu {
  }
  win32 {
  }
}
QMAKE_CLEAN	+= ARADO
QMAKE_CXXFLAGS  += -Wall 
DEFINES         += DELIBERATE_DEBUG=0
ICON		 = images/arado-logo-colo-128.png
win32 {
  # LIBS            += -lws2_32
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
} else {
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
}

TARGET		= bin/arado-post
PROJECTNAME	= ARADO


UI_DIR = tmp/ui
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
OBJECTS_DIR = tmp/obj

include ("translate.pri")

HEADERS = src/arado-post.h \
          src/version.h \
          src/deliberate.h \
          src/cmdoptions.h \

SOURCES = src/postmain.cpp \
          src/arado-post.cpp \
          src/version.cpp \
          src/deliberate.cpp \
          src/cmdoptions.cpp \
