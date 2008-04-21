#include <ruby.h>

#include <QString>
#include <QFileInfo>

#include <KStandardDirs>
#include <KComponentData>
#include <kdebug.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s application_name ruby_script\n", argv[0]);
        return 1;
    }

    KComponentData componentData(argv[1]);
    QString path = componentData.dirs()->locate("data", argv[2]);

    if (path.isEmpty()) {
        kWarning() << "Ruby script" << argv[1] << "missing";
        return 1;
    }

    QFileInfo program(path);
     
    char ** rubyargs = (char **) calloc(argc-1, sizeof(char *));
    rubyargs[0] = strdup(program.fileName().latin1());
    rubyargs[1] = strdup(program.fileName().latin1());
    for (int i = 3; i < argc; i++) {
        rubyargs[i-1] = argv[i];
    }

    RUBY_INIT_STACK
    ruby_init();
    ruby_options(argc-1, rubyargs); 
    ruby_init_loadpath();
    ruby_incpush(QFile::encodeName(program.path()));
    rb_gv_set("$KCODE", rb_str_new2("u"));
    ruby_run();
}
