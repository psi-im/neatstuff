TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= xdata

SOURCES += \
	main.cpp \
	xmpp_xdata.cpp \
	xdata_widget.cpp
HEADERS += \
	xmpp_xdata.h \
	xdata_widget.h

# XMPP
SOURCES += \
	xmpp_jid.cpp
HEADERS += \
	xmpp_jid.h


#INTERFACES += \
#	select.ui \

INCLUDEPATH += ../../xmpp/src
VPATH += ../../xmpp/src
