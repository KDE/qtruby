INCLUDES = $(all_includes) -I $(RUBY_ARCHDIR)

noinst_HEADERS = qtruby.h marshall.h smokeruby.h handlers.cpp Qt.cpp extconf.rb 
noinst_LTLIBRARIES = libqtruby.la

libqtruby_la_LDFLAGS = -version-info 0:0:0
libqtruby_la_METASOURCES = AUTO
libqtruby_la_SOURCES = Qt.cpp handlers.cpp 

SUBDIRS = lib 

install-data-local:
	cp Qt.so $(RUBY_ARCHDIR)
