TEMPLATE	= lib
CONFIG		+= qt thread warn_on release plugin
TARGET		= psiwidgets

target.path	= $(QTDIR)/plugins/designer
INSTALLS	+= target

SOURCES += \
	psiwidgets.cpp \
	fancylabel.cpp \
	busywidget.cpp \
	iconwidget.cpp \
	psitextview.cpp

HEADERS += \
	psiwidgets.h \
	fancylabel.h \
	busywidget.h \
	iconwidget.h \
	psitextview.h

DISTFILES += \
	README \
	iconselect.cpp \
	iconselect.h

DEFINES += WIDGET_PLUGIN
