require 'mkmf'
dir_config('qtruby')
dir_config('smoke')
dir_config('qt')
$LOCAL_LIBS += '-lqt-mt -lqui -lstdc++'
create_makefile("qui")
