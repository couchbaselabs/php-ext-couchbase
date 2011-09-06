#include "php.h"
#include <libcouchbase/couchbase.h>

#ifndef PHP_COUCHBASE_H
#define PHP_COUCHBASE_H 1

#define PHP_COUCHBASE_VERSION "0.0.1"
#define PHP_COUCHBASE_EXTNAME "couchbase"

PHP_FUNCTION(couchbase_version);
PHP_FUNCTION(couchbase_create);
PHP_FUNCTION(couchbase_execute);
PHP_FUNCTION(couchbase_mget);
PHP_FUNCTION(couchbase_set);
PHP_FUNCTION(couchbase_add);
PHP_FUNCTION(couchbase_remove);

// callbacks
PHP_FUNCTION(couchbase_set_storage_callback);
PHP_FUNCTION(couchbase_set_get_callback);

typedef struct _php_couchbase_instance {
  libcouchbase_t instance;
  void *evbase;
} php_couchbase_instance;

typedef struct _php_couchbase_callbacks {
    zval *storage;
    zval *get;
} php_couchbase_callbacks;

#define PHP_COUCHBASE_INSTANCE "Couchbase Instance"

PHP_MINIT_FUNCTION(couchbase);

extern zend_module_entry couchbase_module_entry;
#define phpext_hello_ptr &couchbase_module_entry

#endif
