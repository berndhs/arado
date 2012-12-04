

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

libspoton.commands = $(MAKE) -C LibSpotOn

TEMPLATE	= app
LANGUAGE	= C++
CONFIG		+= qt warn_on build_all
QT		+= network sql xml webkit

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
QMAKE_CLEAN	+= ARADO LibSpotOn/*.o LibSpotOn/*.so LibSpotOn/test
QMAKE_CXXFLAGS  += -Wall 
QMAKE_EXTRA_TARGETS += libspoton
PRE_TARGETDEPS = libspoton
DEFINES         += DELIBERATE_DEBUG=0
ICON		 = images/arado-logo-colo-128.png
win32 {
  # LIBS            += -lws2_32
  LIBS += -LLibSpotOn -LLibSpotOn\Libraries.win32 \
          -lgcrypt-11 -lpthread -lspoton
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
  INCLUDEPATH += LibSpotOn LibSpotOn\\Include.win32
} else {
  LIBS += -LLibSpotOn -lgcrypt -lspoton
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
  INCLUDEPATH += LibSpotOn
}

TARGET		= bin/arado-engine
PROJECTNAME	= ARADO


RESOURCES        = arado.qrc

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
OBJECTS_DIR = tmp/obj

FORMS		=	ui/DebugLog.ui \

include ("translate.pri")

HEADERS = src/arado-engine.h \
          src/version.h \
          src/deliberate.h \
          src/delib-debug.h \
          src/cmdoptions.h \
          src/db-manager.h \
          src/delib-rng.h \
          src/arado-peer.h \
          src/arado-feed.h \
          src/arado-url.h \
          src/rss-poll.h \
          src/addfeed.h \
          src/policy.h \
          src/http-server.h \
          src/http-client.h \
          src/http-sender.h \
          src/poll-sequence.h \
          src/http-client-reply.h \
          src/http-types.h \
          src/arado-peer.h \
          src/networkaccessmanager.h \
          src/arado-stream-parser.h \
          src/control-message.h \
          src/peer-sweeper.h \

SOURCES = src/enginemain.cpp \
          src/arado-engine.cpp \
          src/version.cpp \
          src/deliberate.cpp \
          src/delib-debug.cpp \
          src/cmdoptions.cpp \
          src/db-manager.cpp \
          src/delib-rng.cpp \
          src/arado-peer.cpp \
          src/arado-feed.cpp \
          src/arado-url.cpp \
          src/rss-poll.cpp \
          src/addfeed.cpp \
          src/policy.cpp \
          src/http-server.cpp \
          src/http-client.cpp \
          src/http-sender.cpp \
          src/poll-sequence.cpp \
          src/http-client-reply.cpp \
          src/networkaccessmanager.cpp \
          src/arado-stream-parser.cpp \
          src/control-message.cpp \
          src/peer-sweeper.cpp \
