INCLUDES = -I$(top_srcdir)/smoke $(all_includes) -I$(RUBY_ARCHDIR)

noinst_HEADERS = qtruby.h marshall.h smokeruby.h handlers.cpp Qt.cpp extconf.rb 

lib_LTLIBRARIES = libqtruby.la

libqtruby_la_LDFLAGS = $(all_libraries) -version-info 0:0:0 -L$(top_srcdir)/smoke/qt/
libqtruby_la_METASOURCES = AUTO
libqtruby_la_SOURCES = Qt.cpp handlers.cpp 
libqtruby_la_LIBADD = $(LIB_QT) $(top_builddir)/smoke/qt/libsmokeqt.la

SUBDIRS = lib 

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(RUBY_SITEARCHDIR)
	cp .libs/libqtruby.so $(DESTDIR)$(RUBY_SITEARCHDIR)/Qt.so
