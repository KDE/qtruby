TEMPLATE	= app
CONFIG		+= qt console warn_on release professional
HEADERS	= uic.h \
		  widgetdatabase.h \
		  domtool.h \
		  parser.h \
		  widgetinterface.h

SOURCES	= main.cpp uic.cpp form.cpp object.cpp \
		   subclassing.cpp embed.cpp\
		  widgetdatabase.cpp  \
		  domtool.cpp \
		  parser.cpp

DEFINES		+= QT_INTERNAL_XML
#include( ../../../src/qt_professional.pri )

TARGET		= rbuic
DEFINES 	+= UIC
DESTDIR		= /usr/bin

#target.path=$$bins.path
#INSTALLS        += target

