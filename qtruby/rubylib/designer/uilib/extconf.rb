require 'mkmf'
dir_config('smoke')
dir_config('qt')
$CPPFLAGS += " -I../../../../smoke -I../../qtruby "
$LOCAL_LIBS += '-bundle_loader ../../qtruby/qtruby.bundle -lsmokeqt -lqui -lqt-mt -lstdc++'
create_makefile("qui")
