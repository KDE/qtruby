
add_subdirectory( Qt )

#Laurent verify it
install( FILES  Qt4.rb Qt3.rb DESTINATION ${LIB_INSTALL_DIR} )



#original Makefile.am contents follow:

#SUBDIRS = Qt 
#
#rubylibdir = $(RUBY_SITEDIR)
#rubylib_DATA = Qt4.rb Qt3.rb
#
#install-data-local:
#	@-rm -f $(RUBY_SITEDIR)/Qt.rb
#	@$(LN_S) $(RUBY_SITEDIR)/Qt4.rb $(RUBY_SITEDIR)/Qt.rb
#
