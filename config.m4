dnl $Id$
dnl config.m4 for extension couchbase

PHP_ARG_WITH(couchbase, for couchbase support,
dnl Make sure that the comment is aligned:
[  --with-couchbase=[DIR]            Set the path to libcouchbase install prefix])

PHP_ARG_ENABLE(couchbase-json, whether to enable json serializer support,
[  --enable-couchbase-json Enable json serializer support], no, no)


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

  dnl PHP_REQUIRE_CXX()
  dnl PHP_ADD_LIBRARY(stdc++, 1, COUCHBASE_SHARED_LIBADD)
  dnl PHP_ADD_LIBRARY(event, 1, COUCHBASE_SHARED_LIBADD)
  PHP_SUBST(COUCHBASE_SHARED_LIBADD)

  PHP_NEW_EXTENSION(couchbase, couchbase.c, $ext_shared)
fi
