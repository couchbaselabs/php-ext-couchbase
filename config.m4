dnl $Id$
dnl config.m4 for extension couchbase

PHP_ARG_WITH(couchbase, for couchbase support,
dnl Make sure that the comment is aligned:
[  --with-couchbase=[DIR]            Set the path to libcouchbase install prefix])

if test "$PHP_COUCHBASE" != "no"; then
  if test -r "$PHP_COUCHBASE/include/libcouchbase/couchbase.h"; then
      COUCHBASE_DIR="$PHP_COUCHBASE"
  else
    dnl # look in system dirs
     AC_MSG_CHECKING([for libcouchbase files in default path])
    for i in /usr /usr/local /opt/local; do
      if test -r "$i/include/libcouchbase/couchbase.h"; then
        COUCHBASE_DIR=$i
        AC_MSG_RESULT(found in $i)
        break
      fi
    done
  fi

  if test -z "$COUCHBASE_DIR"; then
     AC_MSG_RESULT([not found])
     AC_MSG_ERROR([couchbase support requires libcouchbase. Use --with-couchbase=<DIR> to specify the prefix where libcouchbase headers and library are located])
  fi

  PHP_ADD_INCLUDE($COUCHBASE_DIR/include/)

  LIBNAME=couchbase # you may want to change this
  LIBSYMBOL=libcouchbase_connect # you most likely want to change this

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $COUCHBASE_DIR/lib, COUCHBASE_SHARED_LIBADD)
    AC_DEFINE(HAVE_COUCHBASELIB,1,[ ])
  ],[
    AC_MSG_ERROR([wrong couchbase lib version or lib not found])
  ],[
    -L$COUCHBASE_DIR/lib -l$LIBNAME
  ])

  dnl PHP_REQUIRE_CXX()
  dnl PHP_ADD_LIBRARY(stdc++, 1, COUCHBASE_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(event, 1, COUCHBASE_SHARED_LIBADD)
  PHP_SUBST(COUCHBASE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(couchbase, couchbase.c, $ext_shared)
fi
