AM_rbFLAGS = -I$(top_srcdir)/kdecore $(all_includes) -DQT_NO_CAST_ASCII

check_PROGRAMS = test1 test2 test3 test4 test5

CLEANFILES = test1.rb test1.h \
             test2.rb test2.h \
             test3.rb test3.h \
             test4.rb test4.h \
             test5.rb test5.h \
             md5sums

test1_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test1_LDADD = $(LIB_KDECORE)
test1_SOURCES = test1main.rb test1.rb

test2_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test2_LDADD = $(LIB_KDECORE)
test2_SOURCES = test2main.rb test2.rb

test3_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test3_LDADD = $(LIB_KDECORE)
test3_SOURCES = test3main.rb test3.rb

test4_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test4_LDADD = $(LIB_KDECORE)
test4_SOURCES = test4main.rb test4.rb

test5_LDFLAGS = $(all_libraries) $(KDE_RPATH)
test5_LDADD = $(LIB_KDECORE)
test5_SOURCES = test5main.rb test5.rb

test1main.o test1.o: test1.h
# avoid running the below command in parallel
test1.rb: test1.h
test1.rb test1.h: $(srcdir)/test1.kcfg ../rbkconfig_compiler $(srcdir)/test1.kcfgc
	../rbkconfig_compiler $(srcdir)/test1.kcfg $(srcdir)/test1.kcfgc

test2main.o test2.o: test2.h
# avoid running the below command in parallel
test2.rb: test2.h
test2.rb test2.h: $(srcdir)/test2.kcfg ../rbkconfig_compiler $(srcdir)/test2.kcfgc
	../rbkconfig_compiler $(srcdir)/test2.kcfg $(srcdir)/test2.kcfgc

test3main.o test3.o: test3.h
# avoid running the below command in parallel
test3.rb: test3.h
test3.rb test3.h: $(srcdir)/test3.kcfg ../rbkconfig_compiler $(srcdir)/test3.kcfgc
	../rbkconfig_compiler $(srcdir)/test3.kcfg $(srcdir)/test3.kcfgc

test4main.o test4.o: test4.h
# avoid running the below command in parallel
test4.rb: test4.h
test4.rb test4.h: $(srcdir)/test4.kcfg ../rbkconfig_compiler $(srcdir)/test4.kcfgc
	../rbkconfig_compiler $(srcdir)/test4.kcfg $(srcdir)/test4.kcfgc

test5main.o test5.o: test5.h
# avoid running the below command in parallel
test5.rb: test5.h
test5.rb test5.h: $(srcdir)/test5.kcfg ../rbkconfig_compiler $(srcdir)/test5.kcfgc
	../rbkconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

md5sums:
	$(MD5SUM) $(srcdir)/test*.ref | sed -e "s,$(srcdir)/,,; s,\.ref$$,," > md5sums

md5check: test1.rb test2.rb test3.rb test4.rb test5.rb md5sums
	$(MD5SUM) --check md5sums

