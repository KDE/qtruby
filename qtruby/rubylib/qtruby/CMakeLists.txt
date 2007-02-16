
add_subdirectory( lib )

IF(QT_QTDBUS_FOUND)
    ADD_DEFINITIONS (-DQT_QTDBUS)
ENDIF(QT_QTDBUS_FOUND)

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${RUBY_INCLUDE_PATH} )
INCLUDE_DIRECTORIES (${QT_INCLUDES})


set(qtruby_LIB_SRCS 
   Qt.cpp 
   handlers.cpp 
   marshall_types.cpp
   )

# Use 'MODULE' instead of 'SHARED' so that qtruby will be built as a bundle on Mac OS X
# instead of a dyld. But is this correct (rjd)? It needs testing.
add_library(qtruby4 MODULE ${qtruby_LIB_SRCS})
#add_library(qtruby4 SHARED ${qtruby_LIB_SRCS})

target_link_libraries(qtruby4 ${QT_QTCORE_LIBRARY} ${RUBY_LIBRARY} ${QT_QTNETWORK_LIBRARY} smokeqt)

set_target_properties(qtruby4 PROPERTIES VERSION 0.0.0 SOVERSION 0 PREFIX "")

install(TARGETS qtruby4  DESTINATION ${RUBY_SITE_ARCH_DIR} )

########### next target ###############


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/smoke $(all_includes) -I$(RUBY_ARCHDIR)
#
#noinst_HEADERS = qtruby.h marshall.h smokeruby.h extconf.rb marshall_types.h \
#		marshall_basetypes.h marshall_complex.h marshall_primitives.h
#
#noinst_LTLIBRARIES = libqtrubyinternal.la
#libqtrubyinternal_la_SOURCES = Qt.cpp handlers.cpp marshall_types.cpp
#libqtrubyinternal_la_METASOURCES = AUTO
#
#rubylibdir = $(RUBY_SITEARCHDIR)
#rubylib_LTLIBRARIES = qtruby4.la
#qtruby4_la_SOURCES = 
#qtruby4_la_LDFLAGS = -module $(all_libraries) -version-info 0:0:0
#qtruby4_la_LIBADD = libqtrubyinternal.la $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QTNETWORK) \
#						$(LIB_QTOPENGL) $(LIB_QTSQL) $(LIB_QTCORE) $(LIB_QTXML) \
# 						$(top_builddir)/smoke/qt/libsmokeqt.la
#
#SUBDIRS = lib 
