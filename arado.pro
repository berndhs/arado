TEMPLATE	= app
LANGUAGE	= C++
CONFIG		+= qt warn_on release
QT		+= gui network
QMAKE_CLEAN	+= ARADO
QMAKE_CXXFLAGS  += -Wall 


TRANS_DIR = translate
TRANSLATIONS += $$TRANS_DIR/arado_de.ts

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
OBJECTS_DIR = tmp/obj

RESOURCES   = arado.qrc

FORMS		=	ui/aradomain.ui \
                        ui/DebugLog.ui \
	                ui/config-edit.ui \

HEADERS		=	src/aradomain.h \
                   src/cmdoptions.h \
                   src/delib-debug.h \
                   src/deliberate.h \
                   src/version.h \
	           src/config-edit.h \


SOURCES		=	src/aradomain.cpp \
			src/main.cpp \
                   src/cmdoptions.cpp \
                   src/delib-debug.cpp \
                   src/deliberate.cpp \   
                   src/version.cpp \
	           src/config-edit.cpp \


TARGET		= bin/arado
PROJECTNAME	= ARADO
