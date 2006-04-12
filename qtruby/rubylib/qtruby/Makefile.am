INCLUDES = -I$(top_srcdir)/smoke $(all_includes) -I$(RUBY_ARCHDIR)

noinst_HEADERS = qtruby.h marshall.h smokeruby.h extconf.rb marshall_types.h \
		marshall_basetypes.h marshall_complex.h marshall_primitives.h

noinst_LTLIBRARIES = libqtrubyinternal.la
libqtrubyinternal_la_SOURCES = Qt.cpp handlers.cpp marshall_types.cpp
libqtrubyinternal_la_METASOURCES = AUTO

rubylibdir = $(RUBY_SITEARCHDIR)
rubylib_LTLIBRARIES = qtruby.la
qtruby_la_SOURCES = 
qtruby_la_LDFLAGS = -module $(all_libraries) -version-info 0:0:0
qtruby_la_LIBADD = libqtrubyinternal.la $(LIB_QTCORE) $(LIB_QTGUI) $(LIB_QTNETWORK) \
						$(LIB_QTOPENGL) $(LIB_QTSQL) $(LIB_QTCORE) $(LIB_QTXML) \
 						$(top_builddir)/smoke/qt/libsmokeqt.la

SUBDIRS = lib 
