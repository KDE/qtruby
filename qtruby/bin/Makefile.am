bin_SCRIPTS = rbqtapi rbqtsh

install-exec-local:
	@-rm -f $(DESTDIR)$(bindir)/rbqt3api
	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt3api
	@-rm -f $(DESTDIR)$(bindir)/rbqt4api
	@$(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt4api

