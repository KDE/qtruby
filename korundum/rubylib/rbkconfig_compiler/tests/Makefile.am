check_PROGRAMS = test1 test2 test3 test4 test5

CLEANFILES = test1.rb \
             test2.rb \
             test3.rb \
             test4.rb \
             test5.rb \
             md5sums

test1_SOURCES = test1main.rb test1.rb
test2_SOURCES = test2main.rb test2.rb
test3_SOURCES = test3main.rb test3.rb
test4_SOURCES = test4main.rb test4.rb
test5_SOURCES = test5main.rb test5.rb

# avoid running the below command in parallel
test1.rb: $(srcdir)/test1.kcfg ../rbkconfig_compiler $(srcdir)/test1.kcfgc
	../rbkconfig_compiler $(srcdir)/test1.kcfg $(srcdir)/test1.kcfgc

# avoid running the below command in parallel
test2.rb: $(srcdir)/test2.kcfg ../rbkconfig_compiler $(srcdir)/test2.kcfgc
	../rbkconfig_compiler $(srcdir)/test2.kcfg $(srcdir)/test2.kcfgc

# avoid running the below command in parallel
test3.rb: $(srcdir)/test3.kcfg ../rbkconfig_compiler $(srcdir)/test3.kcfgc
	../rbkconfig_compiler $(srcdir)/test3.kcfg $(srcdir)/test3.kcfgc

# avoid running the below command in parallel
test4.rb: $(srcdir)/test4.kcfg ../rbkconfig_compiler $(srcdir)/test4.kcfgc
	../rbkconfig_compiler $(srcdir)/test4.kcfg $(srcdir)/test4.kcfgc

# avoid running the below command in parallel
test5.rb: $(srcdir)/test5.kcfg ../rbkconfig_compiler $(srcdir)/test5.kcfgc
	../rbkconfig_compiler $(srcdir)/test5.kcfg $(srcdir)/test5.kcfgc

md5sums:
	$(MD5SUM) $(srcdir)/test*.ref | sed -e "s,$(srcdir)/,,; s,\.ref$$,," > md5sums

md5check: test1.rb test2.rb test3.rb test4.rb test5.rb md5sums
	$(MD5SUM) --check md5sums

