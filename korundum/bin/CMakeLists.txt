
include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES}  )


########### next target ###############

set(krubyinit_SRCS krubyinit.cpp )

kde4_automoc(${krubyinit_SRCS})

kde4_add_executable(krubyinit ${krubyinit_SRCS})

target_link_libraries(krubyinit  ${KDE4_KDECORE_LIBS} smokekde qtruby4 )

install(TARGETS krubyinit DESTINATION ${BIN_INSTALL_DIR})


#install-exec-local:
#	@-rm -f $(DESTDIR)$(bindir)/rbkdeapi
#	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbkdeapi
#	@-rm -f $(DESTDIR)$(bindir)/rbkdesh
#	@$(LN_S) rbqtsh $(DESTDIR)$(bindir)/rbkdesh
