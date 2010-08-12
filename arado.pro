TEMPLATE	= app
LANGUAGE	= C++
CONFIG		+= qt warn_on release
QT		+= gui network
QMAKE_CLEAN	+= ARADO
QMAKE_CXXFLAGS  += -Wall -Werror

UI_DIR = tmp/ui
MOC_DIR = tmp/moc
RCC_DIR = tmp/rcc
OBJECTS_DIR = tmp/obj

FORMS		=	ui/aradomain.ui \

HEADERS		=	src/aradomain.h \


SOURCES		=	src/aradomain.cpp \
			src/main.cpp


TARGET		= bin/arado
PROJECTNAME	= ARADO
