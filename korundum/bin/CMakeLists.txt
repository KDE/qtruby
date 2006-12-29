
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(krubyinit_SRCS krubyinit.cpp )

kde4_automoc(${krubyinit_SRCS})

kde4_add_executable(krubyinit ${krubyinit_SRCS})

target_link_libraries(krubyinit  ${KDE4_KDECORE_LIBS} smokekde qtrubyinternal )

install(TARGETS krubyinit DESTINATION ${BIN_INSTALL_DIR})


########### install files ###############




#original Makefile.am contents follow:

#INCLUDES = $(all_includes) -I$(RUBY_ARCHDIR)
#
#bin_PROGRAMS = krubyinit
#krubyinit_LDFLAGS = -module $(all_libraries) -version-info 0:0:0 -L$(top_srcdir)/smoke/kde/ -L$(RUBY_LIBDIR)
#krubyinit_METASOURCES = AUTO
#krubyinit_SOURCES = krubyinit.cpp
#krubyinit_LDADD = $(LIB_KDE) $(RUBY_LIBRUBYARG) $(top_builddir)/smoke/kde/libsmokekde.la $(top_builddir)/qtruby/rubylib/qtruby/libqtrubyinternal.la
#
#install-exec-local:
#	@-rm -f $(DESTDIR)$(bindir)/rbkdeapi
#	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbkdeapi
#	@-rm -f $(DESTDIR)$(bindir)/rbkdesh
#	@$(LN_S) rbqtsh $(DESTDIR)$(bindir)/rbkdesh
