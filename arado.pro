

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
QT		+= gui network sql xml
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
DEFINES         += DELIBERATE_DEBUG=1
ICON		 = images/arado-logo-colo-128.png
win32 {
  # LIBS            += -lws2_32
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
} else {
  CONFIG -= use_miniupnp
  DEFINES += USE_MINIUPNP=0
}

TRANS_DIR = translate
TRANSLATIONS += $$TRANS_DIR/arado_de.ts \
                $$TRANS_DIR/arado_es.ts \
                $$TRANS_DIR/arado_zh-CN-simple.ts \
                $$TRANS_DIR/arado_zh_TW.ts \
                $$TRANS_DIR/arado_fr.ts \
                $$TRANS_DIR/arado_sv.ts \
                $$TRANS_DIR/arado_af.ts \
                $$TRANS_DIR/arado_ru.ts \
                $$TRANS_DIR/arado_it.ts \
                $$TRANS_DIR/arado_ja.ts \
                $$TRANS_DIR/arado_nl.ts \
                $$TRANS_DIR/arado_pl.ts \
                $$TRANS_DIR/arado_pt.ts \
                $$TRANS_DIR/arado_hy-AM.ts \
                $$TRANS_DIR/arado_az.ts \
                $$TRANS_DIR/arado_eu-ES.ts \
                $$TRANS_DIR/arado_bg-BG.ts \
                $$TRANS_DIR/arado_eu-ES.ts \
                $$TRANS_DIR/arado_da-DK.ts \
                $$TRANS_DIR/arado_et-EE.ts \
                $$TRANS_DIR/arado_fi-FI.ts \
                $$TRANS_DIR/arado_gl.ts \
                $$TRANS_DIR/arado_ka-GE.ts \
                $$TRANS_DIR/arado_el-GR.ts \
                $$TRANS_DIR/arado_he.ts \
                $$TRANS_DIR/arado_hi-IN.ts \
                $$TRANS_DIR/arado_id-ID.ts \
                $$TRANS_DIR/arado_ie-GA.ts \
                $$TRANS_DIR/arado_is.ts \
                $$TRANS_DIR/arado_yi.ts \
                $$TRANS_DIR/arado_ca.ts \
                $$TRANS_DIR/arado_ko-KR.ts \
                $$TRANS_DIR/arado_hr.ts \
                $$TRANS_DIR/arado_lv.ts \
                $$TRANS_DIR/arado_lt.ts \
                $$TRANS_DIR/arado_ms-MY.ts \
                $$TRANS_DIR/arado_mt.ts \
                $$TRANS_DIR/arado_mk-MK.ts \
                $$TRANS_DIR/arado_no.ts \
                $$TRANS_DIR/arado_ro-RO.ts \
                $$TRANS_DIR/arado_sr.ts \
                $$TRANS_DIR/arado_ro-RO.ts \
                $$TRANS_DIR/arado_sk-SK.ts \
                $$TRANS_DIR/arado_sl-SL.ts \
                $$TRANS_DIR/arado_sw.ts \
                $$TRANS_DIR/arado_tl.ts \
                $$TRANS_DIR/arado_th-TH.ts \
                $$TRANS_DIR/arado_cs.ts \
                $$TRANS_DIR/arado_tr-TR.ts \
                $$TRANS_DIR/arado_uk-UA.ts \
                $$TRANS_DIR/arado_sk-SK.ts \
                $$TRANS_DIR/arado_hu.ts \
                $$TRANS_DIR/arado_vi.ts \
                $$TRANS_DIR/arado_be.ts \

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
OBJECTS_DIR = tmp/obj

RESOURCES        = arado.qrc
RC_FILE          = arado.rc

FORMS		=	ui/aradomain.ui \
                        ui/DebugLog.ui \
	                ui/config-edit.ui \
                        ui/url-display.ui \
                        ui/connection-display.ui \
                        ui/entry-form.ui \
                        ui/address-input.ui \
                        ui/listener-edit.ui \
                        ui/rss-list.ui \
                        ui/crawler-settings.ui \

HEADERS		=	src/aradomain.h \
                   src/cmdoptions.h \
                   src/delib-debug.h \
                   src/deliberate.h \
                   src/version.h \
	           src/config-edit.h \
		   src/arado-address.h \
                   src/arado-url.h \
                   src/comm-base.h \
                   src/collector-base.h \
                   src/file-comm.h \
                   src/arado-stream-parser.h \
                   src/file-buffer.h \
                   src/db-manager.h \
                   src/url-display.h \
                   src/connection-display.h \
                   src/entry-form.h \
                   src/policy.h \
                   src/search.h \
                   src/http-server.h \
                   src/http-client.h \
                   src/http-sender.h \
                   src/control-message.h \
                   src/add-peer.h \
                   src/arado-peer.h \
                   src/poll-sequence.h \
                   src/http-client-reply.h \
                   src/http-types.h \
                   src/listener-edit.h \
                   src/addfeed.h \
                   src/rss-list.h \
                   src/rss-poll.h \
                   src/networkaccessmanager.h \
                   src/arado-feed.h \
                   src/crawler-settings.h \
                   src/delib-rng.h \

use_miniupnp {
HEADERS += \
                   src/miniupnp/bsdqueue.h \
                   src/miniupnp/codelength.h \
                   src/miniupnp/connecthostport.h \
                   src/miniupnp/declspec.h \
                   src/miniupnp/igd_desc_parse.h \
                   src/miniupnp/minisoap.h \
                   src/miniupnp/minissdpc.h \
                   src/miniupnp/miniupnpc.h \
                   src/miniupnp/miniupnpcstrings.h \
                   src/miniupnp/miniwget.h \
                   src/miniupnp/minixml.h \
                   src/miniupnp/upnpcommands.h \
                   src/miniupnp/upnperrors.h \
                   src/miniupnp/upnpreplyparse.h \
                   src/upnpclient.h 

}


SOURCES		=	src/aradomain.cpp \
			src/main.cpp \
                   src/cmdoptions.cpp \
                   src/delib-debug.cpp \
                   src/deliberate.cpp \   
                   src/version.cpp \
	           src/config-edit.cpp \
                   src/arado-address.cpp \
                   src/arado-url.cpp \
                   src/comm-base.cpp \
                   src/collector-base.cpp \
                   src/file-comm.cpp \
                   src/arado-stream-parser.cpp \
                   src/file-buffer.cpp \
                   src/db-manager.cpp \
                   src/url-display.cpp \
                   src/connection-display.cpp \
                   src/entry-form.cpp \
                   src/policy.cpp \
                   src/search.cpp \
                   src/http-server.cpp \
                   src/http-client.cpp \
                   src/http-sender.cpp \
                   src/control-message.cpp \
                   src/add-peer.cpp \
                   src/arado-peer.cpp \
                   src/poll-sequence.cpp \
                   src/http-client-reply.cpp \
                   src/listener-edit.cpp \
                   src/addfeed.cpp \
                   src/rss-list.cpp \
                   src/rss-poll.cpp \
                   src/arado-feed.cpp \
                   src/networkaccessmanager.cpp \
                   src/crawler-settings.cpp \
                   src/delib-rng.cpp \

use_miniupnp {
SOURCES +=         \
                   src/miniupnp/miniupnpc.c \
                   src/miniupnp/igd_desc_parse.c \
                   src/miniupnp/minixml.c \
                   src/miniupnp/minisoap.c \
                   src/miniupnp/miniwget.c \
                   src/miniupnp/upnpcommands.c \
                   src/miniupnp/upnperrors.c \
                   src/miniupnp/minissdpc.c \
                   src/miniupnp/connecthostport.c \
                   src/miniupnp/upnpreplyparse.c \
                   src/upnpclient.cpp
}



TARGET		= bin/arado
PROJECTNAME	= ARADO
