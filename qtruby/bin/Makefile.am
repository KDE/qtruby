INCLUDES = $(all_includes) -I$(RUBY_ARCHDIR)

bin_PROGRAMS = qrubyinit
qrubyinit_LDFLAGS = -module $(all_libraries) -version-info 0:0:0 -L$(top_srcdir)/smoke/qt/ -L$(RUBY_LIBDIR)
qrubyinit_METASOURCES = AUTO
qrubyinit_SOURCES = qrubyinit.cpp
qrubyinit_LDADD = $(LIB_QT) $(RUBY_LIBRUBYARG) $(top_builddir)/smoke/qt/libsmokeqt.la $(top_builddir)/qtruby/rubylib/qtruby/libqtrubyinternal.la

bin_SCRIPTS = rbqtapi rbqtsh
