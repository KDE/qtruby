MESSAGE( STATUS "Fix $(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt3api and $(LN_S) rbqtapi $(DESTDIR)$(bindir)/rbqt4api")
install( PROGRAMS  rbqtapi rbqtsh DESTINATION ${CMAKE_INSTALL_PREFIX}/bin )
