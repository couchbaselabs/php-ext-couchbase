PHP_ARG_WITH([couchbase], [for Couchbase support],
             [  --with-couchbase=[DIR]    Include Couchbase support])

if test "$PHP_COUCHBASE" != "no"; then
  AC_CHECK_HEADER([libcouchbase/couchbase.h])
  AS_IF([test "x$ac_cv_header_libcouchbase_couchbase" = "xno"], [
         AC_MSG_ERROR([The Couchbase extension require libcouchbase])])

  PHP_ADD_LIBRARY(couchbase, 1, COUCHBASE_SHARED_LIBADD)

  dnl The JSON api is part of the core as of 5.2
  AC_DEFINE(HAVE_JSON_API,1,[Whether JSON API is available])

  AC_CHECK_HEADER([zlib.h])
  AS_IF([test "x$ac_cv_header_zlib_h" = "xyes"], [
        AC_DEFINE(HAVE_COMPRESSION_ZLIB,1,[Whether zlib lib is available])
        PHP_ADD_LIBRARY(z, 1, COUCHBASE_SHARED_LIBADD)])

  PHP_SUBST(COUCHBASE_SHARED_LIBADD)
  PHP_NEW_EXTENSION([couchbase],
                    [ \
                     apidecl.c \
                     arithmetic.c \
                     ccache.c \
                     compress.c \
                     convert.c \
                     couchbase.c \
                     create.c \
                     designdoc.c \
                     error.c \
                     exceptions.c \
                     fastlz/fastlz.c \
                     flush.c \
                     get.c \
                     ht.c \
                     management/buckets.c \
                     management/instance.c \
                     management/management.c \
                     misc.c  \
                     observe.c \
                     options.c \
                     remove.c \
                     resmgr.c \
                     simple_observe.c \
                     stat.c \
                     store.c \
                     timeout.c \
                     touch.c \
                     unlock.c \
                     version.c \
                     viewopts.c \
                     views.c \
                    ], [$ext_shared])
    PHP_ADD_BUILD_DIR($ext_builddir/fastlz, 1)
    PHP_ADD_BUILD_DIR($ext_builddir/management, 1)
fi
