TEMPLATE	= app
CONFIG		+= qt thread warn_on debug
TARGET		= main

SOURCES += \
	options.cpp \
	main.cpp

HEADERS += \
	options.h

# cutestuff stuff
SOURCES += \
	base64.cpp
HEADERS += \
	base64.h

INCLUDEPATH += ../../cutestuff/util
VPATH += ../../cutestuff/util
