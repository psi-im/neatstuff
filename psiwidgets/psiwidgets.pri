psiwidgets {
	INCLUDEPATH += $$WIDGETS_CPP

	SOURCES += \
		$$WIDGETS_CPP/busywidget.cpp \
		$$WIDGETS_CPP/fancylabel.cpp \
		$$WIDGETS_CPP/iconselect.cpp \
		$$WIDGETS_CPP/iconwidget.cpp \
		$$WIDGETS_CPP/psitextview.cpp

	HEADERS += \
		$$WIDGETS_CPP/busywidget.h \
		$$WIDGETS_CPP/fancylabel.h \
		$$WIDGETS_CPP/iconselect.h \
		$$WIDGETS_CPP/iconwidget.h \
		$$WIDGETS_CPP/psitextview.h

	# to remove dependency on iconset and stuff
	#DEFINES += WIDGET_PLUGIN

	# where to search for widgets plugin
	QMAKE_UIC = $(QTDIR)/bin/uic -L $$WIDGETS_CPP
}
