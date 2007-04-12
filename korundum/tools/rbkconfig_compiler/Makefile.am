SUBDIRS = tests

RBUIC=rbuic

%.rb: %.ui
	$(RBUIC) -tr ${UIC_TR} -kde $*.ui -o $@

exampleprefs_base.rb: $(srcdir)/example.kcfg rbkconfig_compiler $(srcdir)/exampleprefs_base.kcfgc
	./rbkconfig_compiler $(srcdir)/example.kcfg $(srcdir)/exampleprefs_base.kcfgc

AM_CPPFLAGS =  -I$(top_srcdir) $(all_includes) -DQT3_SUPPORT

bin_PROGRAMS = rbkconfig_compiler

rbkconfig_compiler_LDFLAGS = $(all_libraries) $(KDE_RPATH)
rbkconfig_compiler_LDADD = $(LIB_KDECORE)
rbkconfig_compiler_SOURCES = rbkconfig_compiler.cpp

METASOURCES = AUTO

TESTFILES = test1.kcfg test2.kcfg test3.kcfg test4.kcfg test5.kcfg

check-local:
	for i in $(TESTFILES); \
	  do xmllint --noout --schema $(srcdir)/kcfg.xsd $(srcdir)/tests/$$i; \
	  perl $(top_srcdir)/korundum/rubylib/rbkconfig_compiler/checkkcfg.pl \
          $(top_srcdir)/korundum/rubylib/rbkconfig_compiler/tests/$$i; done
	  
