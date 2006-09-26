INCLUDES = $(all_includes) -I$(RUBY_ARCHDIR)

bin_PROGRAMS = qtrubyinit
qtrubyinit_LDFLAGS = -module $(all_libraries) -version-info 0:0:0 -L$(top_srcdir)/smoke/qt/ -L$(RUBY_LIBDIR)
qtrubyinit_METASOURCES = AUTO
qtrubyinit_SOURCES = qtrubyinit.cpp
qtrubyinit_LDADD = $(LIB_QT) $(RUBY_LIBRUBYARG) $(top_builddir)/smoke/qt/libsmokeqt.la $(top_builddir)/qtruby/rubylib/qtruby/libqtrubyinternal.la

bin_SCRIPTS = rbqtapi rbqtsh

install-exec-local:
	@-rm -f $(DESTDIR)$(bindir)/rbqt3api
	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt3api
	@-rm -f $(DESTDIR)$(bindir)/rbqt4api
	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt4api

