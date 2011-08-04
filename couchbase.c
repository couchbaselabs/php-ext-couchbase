#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_couchbase.h"

static function_entry couchbase_functions[] = {
    PHP_FE(couchbase_hello, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry couchbase_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_COUCHBASE_EXTNAME,
    couchbase_functions,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
#if ZEND_MODULE_API_NO >= 20010901
    PHP_COUCHBASE_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_COUCHBASE
ZEND_GET_MODULE(couchbase)
#endif

PHP_FUNCTION(couchbase_hello)
{
    RETURN_STRING("Hello Couchbase", 1);
}
