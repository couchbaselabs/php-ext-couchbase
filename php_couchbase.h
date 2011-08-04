#ifndef PHP_COUCHBASE_H
#define PHP_COUCHBASE_H 1

#define PHP_COUCHBASE_VERSION "0.0.1"
#define PHP_COUCHBASE_EXTNAME "couchbase"

PHP_FUNCTION(couchbase_hello);

extern zend_module_entry couchbase_module_entry;
#define phpext_hello_ptr &couchbase_module_entry

#endif