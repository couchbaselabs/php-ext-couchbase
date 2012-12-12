dnl $Id$
dnl config.m4 for extension couchbase

PHP_ARG_WITH(couchbase, for couchbase support,
dnl Make sure that the comment is aligned:
[  --with-couchbase=[DIR]            Set the path to libcouchbase install prefix])

PHP_ARG_ENABLE(couchbase-json, whether to enable json serializer support,
[  --disable-couchbase-json Disable json serializer support], no)

if test -z "$PHP_ZLIB_DIR"; then
PHP_ARG_WITH(zlib-dir, if zlib directory specified,
[  --with-zlib-dir[=DIR]   Set the path to ZLIB install prefix.], no, no)
fi

if test "$PHP_COUCHBASE" != "no"; then
  if test -r "$PHP_COUCHBASE/include/libcouchbase/couchbase.h"; then
      AC_MSG_CHECKING([for libcouchbase location])
      COUCHBASE_DIR="$PHP_COUCHBASE"
      AC_MSG_RESULT($PHP_COUCHBASE)
  else
    dnl # look in system dirs
    AC_MSG_CHECKING([for libcouchbase files in default path])
    for i in /usr /usr/pkg /opt/local /usr/local; do
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
  LIBSYMBOL=lcb_connect # you most likely want to change this
  BADSYMBOL=libcouchbase_connect
  ERRMSG="
        It appears you are using libcouchbase 1.x
        This DP version of the php extension supports libcouchbase-2.0.0beta
        or higher. Either use a stable 1.0 version of the php extension,
        upgrade your libcouchbase, or specify the location of a
        libcouchbase-2.0.0 installation with --with-couchbase"

  PHP_CHECK_LIBRARY($LIBNAME, $BADSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $COUCHBASE_DIR/lib, COUCHBASE_SHARED_LIBADD)
    HAVE_VERSION_MISMATCH="yes"
  ], [])

  PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  [
    PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $COUCHBASE_DIR/lib, COUCHBASE_SHARED_LIBADD)
    AC_DEFINE(HAVE_COUCHBASELIB,1,[ ])
  ],[
    if test -n "$HAVE_VERSION_MISMATCH"; then
      AC_MSG_ERROR([$ERRMSG])
    fi
    AC_MSG_ERROR([libcouchbase not found])
  ],[
    -L$COUCHBASE_DIR/lib -l$LIBNAME
  ])

  if test "$PHP_COUCHBASE_JSON" != "no"; then
    AC_MSG_CHECKING([for json includes])
    json_inc_path=""

    tmp_version=$PHP_VERSION
    if test -z "$tmp_version"; then
      if test -z "$PHP_CONFIG"; then
        AC_MSG_ERROR([php-config not found])
      fi
      PHP_VERSION_ORIG=`$PHP_CONFIG --version`;
    else
      PHP_VERSION_ORIG=$tmp_version
    fi

   if test -z $PHP_VERSION_ORIG; then
     AC_MSG_ERROR([failed to detect PHP version, please report])
   fi

   PHP_VERSION_MASK=`echo ${PHP_VERSION_ORIG} | awk 'BEGIN { FS = "."; } { printf "%d", ($1 * 1000 + $2) * 1000 + $3;}'`

   if test $PHP_VERSION_MASK -ge 5003000; then
     if test -f "$abs_srcdir/include/php/ext/json/php_json.h"; then
       json_inc_path="$abs_srcdir/include/php"
     elif test -f "$abs_srcdir/ext/json/php_json.h"; then
       json_inc_path="$abs_srcdir"
     elif test -f "$phpincludedir/ext/json/php_json.h"; then
       json_inc_path="$phpincludedir"
     else
       for i in php php4 php5 php6; do
         if test -f "$prefix/include/$i/ext/json/php_json.h"; then
           json_inc_path="$prefix/include/$i"
         fi
       done
     fi
     if test "$json_inc_path" = ""; then
       AC_MSG_ERROR([Cannot find php_json.h])
     else
       AC_DEFINE(HAVE_JSON_API,1,[Whether JSON API is available])
       AC_DEFINE(HAVE_JSON_API_5_3,1,[Whether JSON API for PHP 5.3 is available])
       AC_MSG_RESULT([$json_inc_path])
     fi
   elif test $PHP_VERSION_MASK -ge 5002009; then
     dnl Check JSON for PHP 5.2.9+
     if test -f "$abs_srcdir/include/php/ext/json/php_json.h"; then
       json_inc_path="$abs_srcdir/include/php"
     elif test -f "$abs_srcdir/ext/json/php_json.h"; then
       json_inc_path="$abs_srcdir"
     elif test -f "$phpincludedir/ext/json/php_json.h"; then
       json_inc_path="$phpincludedir"
     else
       for i in php php4 php5 php6; do
         if test -f "$prefix/include/$i/ext/json/php_json.h"; then
           json_inc_path="$prefix/include/$i"
         fi
       done
     fi
     if test "$json_inc_path" = ""; then
       AC_MSG_ERROR([Cannot find php_json.h])
     else
       AC_DEFINE(HAVE_JSON_API,1,[Whether JSON API is available])
       AC_DEFINE(HAVE_JSON_API_5_2,1,[Whether JSON API for PHP 5.2 is available])
       AC_MSG_RESULT([$json_inc_path])
     fi
   else
     AC_MSG_ERROR([the PHP version does not support JSON serialization API])
   fi
  fi

  AC_MSG_CHECKING([for zlib location])
  if test "$PHP_ZLIB_DIR" != "no" && test "$PHP_ZLIB_DIR" != "yes"; then
    if test -f "$PHP_ZLIB_DIR/include/zlib/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include/zlib"
    elif test -f "$PHP_ZLIB_DIR/include/zlib.h"; then
      PHP_ZLIB_DIR="$PHP_ZLIB_DIR"
      PHP_ZLIB_INCDIR="$PHP_ZLIB_DIR/include"
    else
      AC_MSG_ERROR([Can't find ZLIB headers under "$PHP_ZLIB_DIR"])
    fi
  else
    for i in /usr /usr/pkg /opt/local /usr/local; do
      if test -f "$i/include/zlib/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include/zlib"
      elif test -f "$i/include/zlib.h"; then
        PHP_ZLIB_DIR="$i"
        PHP_ZLIB_INCDIR="$i/include"
      fi
    done
  fi
  AC_MSG_RESULT($PHP_ZLIB_DIR)

  if test "$PHP_ZLIB_DIR" != "no" && test "$PHP_ZLIB_DIR" != "yes"; then
    PHP_ADD_LIBRARY_WITH_PATH(z, $PHP_ZLIB_DIR/$PHP_LIBDIR, COUCHBASE_SHARED_LIBADD)
    PHP_ADD_INCLUDE($PHP_ZLIB_INCDIR)
    AC_DEFINE(HAVE_COMPRESSION_ZLIB,1,[Whether zlib lib is available])
  else
    AC_MSG_ERROR([couchbase support requires ZLIB. Use --with-zlib-dir=<DIR> to specify the prefix where ZLIB headers and library are located])
  fi

  dnl PHP_REQUIRE_CXX()
  dnl PHP_ADD_LIBRARY(stdc++, 1, COUCHBASE_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(event, 1, COUCHBASE_SHARED_LIBADD)
  PHP_SUBST(COUCHBASE_SHARED_LIBADD)
  PHP_NEW_EXTENSION([couchbase],
                    [ \
                     apidecl.c \
                     arithmetic.c \
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
fi
