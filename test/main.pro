TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= test

SOURCES += \
	main.cpp
#HEADERS += \

# iconset
#SOURCES += \
#	iconset.cpp \
#	psipng.cpp \
#	anim.cpp
#HEADERS += \
#	iconset.h \
#	psipng.h \
#	anim.h

# Required widgets
#SOURCES += \
#	fancylabel.cpp \
#	busywidget.cpp \
#	psitextview.cpp \
#	iconwidget.cpp \
#	iconselect.cpp
#HEADERS += \
#	fancylabel.h \
#	busywidget.h \
#	psitextview.h \
#	iconwidget.h \
#	iconselect.h

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

