INCLUDES = -I$(top_srcdir)/smoke -I$(top_srcdir)/qtruby/rubylib/qtruby $(all_includes) -I$(RUBY_ARCHDIR)

rubylibdir = $(RUBY_SITEARCHDIR)
rubylib_LTLIBRARIES = qui.la
qui_la_SOURCES = qui.cpp
qui_la_LDFLAGS = -module -export-dynamic $(all_libraries) -version-info 0:0:0
qui_la_LIBADD = -lqui