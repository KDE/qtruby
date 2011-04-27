#ifndef QTRUBY_EXPORT_H
#define QTRUBY_EXPORT_H

#if defined(BUILD_QTRUBY)
#  define QTRUBY_EXPORT Q_DECL_EXPORT
#else
#  define QTRUBY_EXPORT Q_DECL_IMPORT
#endif

#endif
