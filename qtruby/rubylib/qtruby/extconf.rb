require 'mkmf'
dir_config('smoke')
dir_config('qt')
$LOCAL_LIBS += '-lsmokeqt -lQtCore -lQtGui -lQtNetwork -lQtOpenGL -lQtSql -lQtXml -lstdc++'
create_makefile("qtruby")
