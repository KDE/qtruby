########### next target ###############

set(rbrcc_SRCS rcc.cpp main.cpp )

kde4_automoc(${rbrcc_SRCS})

kde4_add_executable(rbrcc ${rbrcc_SRCS})

target_link_libraries(rbrcc  ${KDE4_KDECORE_LIBS} )

install(TARGETS rbrcc DESTINATION ${BIN_INSTALL_DIR})


########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#AM_CPPFLAGS = 
#AM_CXXFLAGS = $(KDE_CXXFLAGS)
#
#bin_PROGRAMS = rbrcc
#noinst_LTLIBRARIES = librbrcc.la
#
#INCLUDES = $(all_includes)
#
#METASOURCES = AUTO
#
#librbrcc_la_SOURCES = rcc.cpp
#
#librbrcc_la_LIBADD  = $(LIBZ) $(LIB_QTCORE) $(LIB_QTXML) 
#
#rbrcc_SOURCES = main.cpp
#rbrcc_LDADD   = librbrcc.la $(all_libraries) 
#rbrcc_LDFLAGS = 
#
#EXTRA_DIST = rcc.cpp rcc.h
#
#messages:
#	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
#	if test -n "$$LIST"; then \
#	 $(XGETTEXT) $$LIST -o $(podir)/rbrcc.pot; \
#	fi
