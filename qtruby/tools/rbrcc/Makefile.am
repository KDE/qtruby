AM_CPPFLAGS = 
AM_CXXFLAGS = $(KDE_CXXFLAGS)

bin_PROGRAMS = rbrcc
noinst_LTLIBRARIES = librbrcc.la

INCLUDES = $(all_includes)

METASOURCES = AUTO

librbrcc_la_SOURCES = rcc.cpp

librbrcc_la_LIBADD  = $(LIBZ) $(LIB_QTCORE) $(LIB_QTXML) 

rbrcc_SOURCES = main.cpp
rbrcc_LDADD   = librbrcc.la $(all_libraries) 
rbrcc_LDFLAGS = 

EXTRA_DIST = rcc.cpp rcc.h

messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/rbrcc.pot; \
	fi
