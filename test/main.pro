TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= test

SOURCES += \
	main.cpp
#HEADERS += \

# misc
SOURCES += \
	zip.cpp \
	minizip/unzip.c

HEADERS += \
	zip.h

INTERFACES += \
	select.ui \
	display.ui

INCLUDEPATH += ../../cutestuff/util
VPATH += ../../cutestuff/util

#INCLUDEPATH += ../psiwidgets
#VPATH += ../psiwidgets

CONFIG += psiwidgets
WIDGETS_CPP = ../psiwidgets
include($$WIDGETS_CPP/psiwidgets.pri)

#INCLUDEPATH += ../iconset
#VPATH += ../iconset

CONFIG += iconset
ICONSET_CPP = ../iconset
include($$ICONSET_CPP/iconset.pri)

INCLUDEPATH += ../misc
VPATH += ../misc

# SHA1 hashing
PSICS_CPP = ../../cutestuff
INCLUDEPATH += \
	$$PSICS_CPP/util
HEADERS += \
	$$PSICS_CPP/util/sha1.h
SOURCES += \
	$$PSICS_CPP/util/sha1.cpp
