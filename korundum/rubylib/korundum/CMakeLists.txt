
add_subdirectory( lib )

include_directories( ${CMAKE_SOURCE_DIR}/smoke ${CMAKE_SOURCE_DIR}/qtruby/rubylib/qtruby ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/smoke -I$(top_srcdir)/qtruby/rubylib/qtruby $(all_includes) -I$(RUBY_ARCHDIR) -DQT3_SUPPORT
#
#rubylibdir = $(RUBY_SITEARCHDIR)
#rubylib_LTLIBRARIES = korundum.la
#korundum_la_LDFLAGS = -module $(all_libraries) -version-info 0:0:0
#korundum_la_METASOURCES = AUTO
#korundum_la_SOURCES = Korundum.cpp kdehandlers.cpp
#korundum_la_LIBADD = $(LIB_KDE) $(top_builddir)/smoke/kde/libsmokekde.la $(top_builddir)/qtruby/rubylib/qtruby/libqtrubyinternal.la
#
#SUBDIRS = lib
