INCLUDES = -I$(top_srcdir)/smoke $(all_includes) -I$(RUBY_ARCHDIR)

noinst_HEADERS = qtruby.h marshall.h smokeruby.h extconf.rb 

noinst_LTLIBRARIES = libqtrubyinternal.la
libqtrubyinternal_la_SOURCES = Qt.cpp handlers.cpp 
libqtrubyinternal_la_METASOURCES = AUTO

rubylibdir = $(RUBY_SITEARCHDIR)
rubylib_LTLIBRARIES = qtruby.la
qtruby_la_SOURCES = 
qtruby_la_LDFLAGS = -module $(all_libraries) -version-info 0:0:0
qtruby_la_LIBADD = libqtrubyinternal.la $(LIB_QT) $(top_builddir)/smoke/qt/libsmokeqt.la

SUBDIRS = lib 
