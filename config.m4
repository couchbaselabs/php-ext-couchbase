PHP_ARG_ENABLE(couchbase, whether to enable Couchbase support,
[  --enable-couchbase   Enable Couchbase support])

PHP_ARG_WITH(libcouchbase-dir,  for libcouchbase,
[  --with-libcouchbase-dir[=DIR]   Set the path to libcouchbase install prefix.], yes)

if test "$PHP_COUCHBASE" = "yes"; then

  if test "$PHP_LIBCOUCHBASE_DIR" != "no" && test "$PHP_LIBCOUCHBASE_DIR" != "yes"; then
    if test -r "$PHP_LIBCOUCHBASE_DIR/include/libcouchbase/couchbase.h"; then
      PHP_LIBCOUCHBASE_DIR="$PHP_LIBCOUCHBASE_DIR"
    else
      AC_MSG_ERROR([Can't find libcouchbase headers under "$PHP_LIBCOUCHBASE_DIR"])
    fi
  else
    dnl # look in system dirs
    PHP_LIBCOUCHBASE_DIR="no"
    for i in /usr /usr/local /opt/local; do
      if test -r "$i/include/libcouchbase/couchbase.h"; then
        PHP_LIBCOUCHBASE_DIR=$i
        break
      fi
    done
  fi

  AC_MSG_CHECKING([for libcouchbase location])
  if test "$PHP_LIBCOUCHBASE_DIR" = "no"; then
    AC_MSG_ERROR([couchbase support requires libcouchbase. Use --with-libcouchbase-dir=<DIR> to specify the prefix where libcouchbase headers and library are located])
  else
    PHP_REQUIRE_CXX
    PHP_ADD_LIBRARY(stdc++, 1, COUCHBASE_SHARED_LIBADD)
    PHP_ADD_LIBRARY(event, 1, COUCHBASE_SHARED_LIBADD)

    PHP_LIBCOUCHBASE_INCDIR="$PHP_LIBCOUCHBASE_DIR/include"

    PHP_ADD_INCLUDE($PHP_LIBCOUCHBASE_INCDIR)
    PHP_ADD_LIBRARY_WITH_PATH(couchbase, $PHP_LIBCOUCHBASE_DIR/$PHP_LIBDIR, COUCHBASE_SHARED_LIBADD)

    PHP_SUBST(COUCHBASE_SHARED_LIBADD)

    AC_DEFINE(HAVE_COUCHBASE, 1, [Whether you have Couchbase])
    PHP_NEW_EXTENSION(couchbase, couchbase.c, $ext_shared)

  fi
fi
