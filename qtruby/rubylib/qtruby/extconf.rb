require 'mkmf'
dir_config('smoke')
dir_config('qt')
$LOCAL_LIBS += '-lsmokeqt -lqt-mt -lstdc++'
create_makefile("Qt")
