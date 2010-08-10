TEMPLATE	= app
LANGUAGE	= C++
CONFIG		+= qt warn_on release
QT		+= gui network
QMAKE_CLEAN	+= ARADO
QMAKE_CXXFLAGS  += -Wall -Werror

FORMS		=	aradomain.ui \

HEADERS		=	aradomain.h \


SOURCES		=	aradomain.cc \


TARGET		= ARADO
PROJECTNAME	= ARADO
