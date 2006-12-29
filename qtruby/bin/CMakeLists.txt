MESSAGE( STATUS "Fix $(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt3api and $(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt4api")
install( PROGRAMS  rbqtapi rbqtsh DESTINATION ${BIN_INSTALL_DIR} )



#original Makefile.am contents follow:

#bin_SCRIPTS = rbqtapi rbqtsh
#
#install-exec-local:
#	@-rm -f $(DESTDIR)$(bindir)/rbqt3api
#	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt3api
#	@-rm -f $(DESTDIR)$(bindir)/rbqt4api
#	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt4api
#
