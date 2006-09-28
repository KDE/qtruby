AM_CPPFLAGS = -DUIC -DQT_INTERNAL_XML
AM_CXXFLAGS = $(KDE_CXXFLAGS)

bin_PROGRAMS = rbuic4
noinst_LTLIBRARIES = librbuic4.la

INCLUDES = $(all_includes) -Iruby

METASOURCES = AUTO

librbuic4_la_SOURCES = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
					validator.cpp ruby/rbwritedeclaration.cpp ruby/rbwriteicondata.cpp ruby/rbwriteicondeclaration.cpp \
					ruby/rbwriteiconinitialization.cpp ruby/rbwriteinitialization.cpp

librbuic4_la_LIBADD  = $(LIBZ) $(LIB_QTCORE) $(LIB_QTXML) 

rbuic4_SOURCES = main.cpp
rbuic4_LDADD   = librbuic4.la $(all_libraries) 
rbuic4_LDFLAGS = 

EXTRA_DIST = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
					validator.cpp writedeclaration.cpp writeicondata.cpp writeicondeclaration.cpp \
					writeiconinitialization.cpp writeinitialization.cpp \
					customwidgetsinfo.h databaseinfo.h driver.h globaldefs.h option.h \
					qclass_lib_map.h treewalker.h ui4.h uic.h utils.h validator.h ruby/rbwritedeclaration.h \
					ruby/rbwriteicondata.h ruby/rbwriteicondeclaration.h ruby/rbwriteiconinitialization.h \
					ruby/rbwriteinitialization.h


messages:
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
	 $(XGETTEXT) $$LIST -o $(podir)/rbuic4.pot; \
	fi
