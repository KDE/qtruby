
include_directories( ${CMAKE_SOURCE_DIR}/qtruby/rubylib/designer/rbuic/ruby/ )
########### next target ###############

set(rbuic4_bin_SRCS 
   customwidgetsinfo.cpp 
   databaseinfo.cpp 
   driver.cpp 
   treewalker.cpp 
   ui4.cpp 
   uic.cpp 
   validator.cpp 
   ruby/rbwritedeclaration.cpp 
   ruby/rbwriteicondata.cpp 
   ruby/rbwriteicondeclaration.cpp 
   ruby/rbwriteiconinitialization.cpp 
   ruby/rbwriteinitialization.cpp 
   main.cpp )

kde4_automoc(${rbuic4_bin_SRCS})

kde4_add_executable(rbuic4_bin ${rbuic4_bin_SRCS})
set_target_properties(rbuic4_bin PROPERTIES OUTPUT_NAME rbuic4)

target_link_libraries(rbuic4_bin  ${KDE4_KDECORE_LIBS} )

install(TARGETS rbuic4_bin DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#AM_CPPFLAGS = -DUIC -DQT_INTERNAL_XML
#AM_CXXFLAGS = $(KDE_CXXFLAGS)
#
#bin_PROGRAMS = rbuic4
#noinst_LTLIBRARIES = librbuic4.la
#
#INCLUDES = $(all_includes) -Iruby
#
#METASOURCES = AUTO
#
#librbuic4_la_SOURCES = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
#					validator.cpp ruby/rbwritedeclaration.cpp ruby/rbwriteicondata.cpp ruby/rbwriteicondeclaration.cpp \
#					ruby/rbwriteiconinitialization.cpp ruby/rbwriteinitialization.cpp
#
#librbuic4_la_LIBADD  = $(LIBZ) $(LIB_QTCORE) $(LIB_QTXML) 
#
#rbuic4_SOURCES = main.cpp
#rbuic4_LDADD   = librbuic4.la $(all_libraries) 
#rbuic4_LDFLAGS = 
#
#EXTRA_DIST = customwidgetsinfo.cpp databaseinfo.cpp driver.cpp treewalker.cpp ui4.cpp uic.cpp \
#					validator.cpp writedeclaration.cpp writeicondata.cpp writeicondeclaration.cpp \
#					writeiconinitialization.cpp writeinitialization.cpp \
#					customwidgetsinfo.h databaseinfo.h driver.h globaldefs.h option.h \
#					qclass_lib_map.h treewalker.h ui4.h uic.h utils.h validator.h ruby/rbwritedeclaration.h \
#					ruby/rbwriteicondata.h ruby/rbwriteicondeclaration.h ruby/rbwriteiconinitialization.h \
#					ruby/rbwriteinitialization.h
#
#
#messages:
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/rbuic4.pot; \
#	fi
