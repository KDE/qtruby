INCLUDES = -I$(top_srcdir)/smoke -I$(top_srcdir)/qtruby/rubylib/qtruby $(all_includes) -I$(RUBY_ARCHDIR)

lib_LTLIBRARIES = libkderuby.la

libkderuby_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -L$(top_srcdir)/smoke/kde/
libkderuby_la_METASOURCES = AUTO
libkderuby_la_SOURCES = Korundum.cpp kdehandlers.cpp Qt.cpp handlers.cpp
libkderuby_la_LIBADD = $(LIB_KDE) $(top_builddir)/smoke/kde/libsmokekde.la

SUBDIRS = lib

install-data-local:
	cp .libs/libkderuby.so $(RUBY_SITEARCHDIR)/Korundum.so
