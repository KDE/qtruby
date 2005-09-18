AM_CPPFLAGS = -DUIC -DQT_INTERNAL_XML
AM_CXXFLAGS = $(KDE_CXXFLAGS)

bin_PROGRAMS = rbuic
noinst_LTLIBRARIES = librbuic.la

INCLUDES = $(all_includes)

METASOURCES = AUTO

librbuic_la_SOURCES = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
					validator.cpp writedeclaration.cpp writeicondata.cpp writeicondeclaration.cpp \
					writeiconinitialization.cpp writeinitialization.cpp

librbuic_la_LIBADD  = $(LIBZ) $(LIB_QTCORE) $(LIB_QTXML) 

rbuic_SOURCES = main.cpp
rbuic_LDADD   = librbuic.la $(all_libraries) 
rbuic_LDFLAGS = 

EXTRA_DIST = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
					validator.cpp writedeclaration.cpp writeicondata.cpp writeicondeclaration.cpp \
					writeiconinitialization.cpp writeinitialization.cpp \
					customwidgetsinfo.h databaseinfo.h driver.h globaldefs.h option.h \
					qclass_lib_map.h treewalker.h ui4.h uic.h utils.h validator.h writedeclaration.h \
					writeicondata.h writeicondeclaration.h writeiconinitialization.h \
					writeinitialization.h


messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/rbuic.pot; \
	fi
