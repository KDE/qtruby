#include "ruby.h"

int main(int argc, char **argv) {
     ruby_init();
     ruby_script("embedded");
     ruby_options(argc, argv);
     ruby_run();
}
