#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "php.h"
#include "php_couchbase.h"
#include <event.h>

int le_couchbase_instance;

static function_entry couchbase_functions[] = {
    PHP_FE(couchbase_version, NULL)
    PHP_FE(couchbase_create, NULL)
    {NULL, NULL, NULL}
};

zend_module_entry couchbase_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_COUCHBASE_EXTNAME,
    couchbase_functions,
    PHP_MINIT(couchbase),
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

// php boilerplate

static void php_couchbase_instance_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_couchbase_instance *php_instance = (php_couchbase_instance*)rsrc->ptr;

    if (php_instance) {
        if (php_instance->instance) {
            libcouchbase_destroy(php_instance->instance);
        }
        efree(php_instance);
    }
}

PHP_MINIT_FUNCTION(couchbase)
{
    le_couchbase_instance = zend_register_list_destructors_ex(php_couchbase_instance_dtor, NULL, PHP_COUCHBASE_INSTANCE, module_number);

    return SUCCESS;
}

// functions

PHP_FUNCTION(couchbase_version)
{
    RETURN_STRING(PHP_COUCHBASE_VERSION, 1);
}

PHP_FUNCTION(couchbase_create)
{
    const char *host = NULL; int host_len;
    const char *user = NULL; int user_len;
    const char *passwd = NULL; int passwd_len;
    const char *bucket = NULL; int bucket_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sss",
        &host, &host_len,
        &user, &user_len,
        &passwd, &passwd_len,
        &bucket, &bucket_len
        ) == FAILURE) {
        RETURN_NULL();
    }

    struct event_base *base;
    struct event_base *evbase = event_init();
    libcouchbase_t instance = libcouchbase_create(host, user,
                                                  passwd, bucket, evbase);
    if (instance == NULL) {
        php_printf("Failed to create libcouchbase instance\n");
        RETURN_FALSE;
    }

    if (libcouchbase_connect(instance) != LIBCOUCHBASE_SUCCESS) {
        php_printf("Failed to connect libcouchbase instance to server\n");
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    php_instance = emalloc(sizeof(php_couchbase_instance));
    php_instance->instance = instance;

    ZEND_REGISTER_RESOURCE(return_value, php_instance, le_couchbase_instance);
}

#ifdef __cplusplus
}
#endif
