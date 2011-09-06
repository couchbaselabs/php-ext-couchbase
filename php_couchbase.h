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
PHP_FUNCTION(couchbase_replace);
PHP_FUNCTION(couchbase_remove);

// callbacks
PHP_FUNCTION(couchbase_set_storage_callback);
PHP_FUNCTION(couchbase_set_get_callback);
PHP_FUNCTION(couchbase_set_remove_callback);

typedef struct _php_couchbase_instance {
  libcouchbase_t instance;
  void *evbase;
} php_couchbase_instance;

typedef struct _php_couchbase_callbacks {
    zval *storage;
    zval *get;
    zval *remove;
} php_couchbase_callbacks;

typedef enum {
    STORAGE_CALLBACK = 1,
    GET_CALLBACK = 2,
    REMOVE_CALLBACK = 3
} php_couchbase_callback_type;

#define PHP_COUCHBASE_INSTANCE "Couchbase Instance"

PHP_MINIT_FUNCTION(couchbase);

// forward declaration
long map_error_constant(libcouchbase_error_t error);
static void couchbase_store(INTERNAL_FUNCTION_PARAMETERS, libcouchbase_storage_t operation);
static void couchbase_set_callback(INTERNAL_FUNCTION_PARAMETERS, php_couchbase_callback_type type);
static void storage_callback(libcouchbase_t instance,
                             const void *cookie,
                             libcouchbase_storage_t operation,
                             libcouchbase_error_t error,
                             const void *key, size_t nkey,
                             uint64_t cas);
static void get_callback(libcouchbase_t instance,
                         const void *cookie,
                         libcouchbase_error_t error,
                         const void *key, size_t nkey,
                         const void *bytes, size_t nbytes,
                         uint32_t flags, uint64_t cas);
static void remove_callback(libcouchbase_t instance,
                     const void *cookie,
                     libcouchbase_error_t error,
                     const void *key, size_t nkey);


extern zend_module_entry couchbase_module_entry;
#define phpext_hello_ptr &couchbase_module_entry

#endif
