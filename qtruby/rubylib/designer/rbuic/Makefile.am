AM_CPPFLAGS = -DUIC -DQT_INTERNAL_XML
AM_CXXFLAGS = $(KDE_CXXFLAGS)

bin_PROGRAMS = rbuic
noinst_LTLIBRARIES = librbuic.la

INCLUDES = $(all_includes)

METASOURCES = AUTO

librbuic_la_SOURCES = widgetdatabase.cpp uic.cpp subclassing.cpp parser.cpp object.cpp form.cpp embed.cpp domtool.cpp
librbuic_la_LIBADD  = $(LIB_QT) $(LIBZ)

rbuic_SOURCES = main.cpp
rbuic_LDADD   = librbuic.la $(all_libraries) 
rbuic_LDFLAGS = $(KDE_RPATH)

EXTRA_DIST = domtool.cpp domtool.h embed.cpp form.cpp globaldefs.h main.cpp object.cpp parser.cpp parser.h subclassing.cpp uic.cpp uic.h widgetdatabase.cpp widgetdatabase.h widgetinterface.h

messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/puic.pot; \
	fi
