TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= popups

SOURCES += \
	main.cpp \
	fancypopup.cpp

HEADERS += \
	fancypopup.h \
	popuplist.h

INTERFACES += \
	dialog.ui


# misc
SOURCES += \
	zip.cpp \
	minizip/unzip.c

HEADERS += \
	zip.h

CONFIG += psiwidgets
WIDGETS_CPP = ../psiwidgets
include($$WIDGETS_CPP/psiwidgets.pri)

CONFIG += iconset
ICONSET_CPP = ../iconset
include($$ICONSET_CPP/iconset.pri)

CONFIG += qpassivepopup
QPASSIVEPOPUP_CPP = ../../cutestuff/qpassivepopup
include($$QPASSIVEPOPUP_CPP/qpassivepopup.pri)

INCLUDEPATH += ../misc
VPATH += ../misc

INCLUDEPATH += ../../cutestuff/util
VPATH += ../../cutestuff/util
