TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= main

SOURCES += \
	vcard.cpp \
	vcardfactory.cpp \
	main.cpp

HEADERS += \
	vcard.h \
	vcardfactory.h \

# cutestuff stuff
SOURCES += \
	base64.cpp \
	jid.cpp
HEADERS += \
	base64.h \
	jid.h

INCLUDEPATH += ../../cutestuff/util
VPATH += ../../cutestuff/util

INCLUDEPATH += ../misc
VPATH += ../misc
