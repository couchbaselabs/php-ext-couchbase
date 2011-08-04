PHP_ARG_ENABLE(couchbase, whether to enable Couchbase support,
[  --enable-couchbase   Enable Couchbase support])

if test "$PHP_COUCHBASE" = "yes"; then
  AC_DEFINE(HAVE_COUCHBASE, 1, [Whether you have Couchbase])
  PHP_NEW_EXTENSION(couchbase, couchbase.c, $ext_shared)
fi
