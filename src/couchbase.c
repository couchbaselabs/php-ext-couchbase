#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_couchbase.h"
#include <event.h>

int le_couchbase_instance;

static zend_function_entry couchbase_functions[] = {
    PHP_FE(couchbase_version, NULL)
    PHP_FE(couchbase_create, NULL)
    PHP_FE(couchbase_execute, NULL)
    PHP_FE(couchbase_mget, NULL)
    PHP_FE(couchbase_set, NULL)
    PHP_FE(couchbase_add, NULL)
    PHP_FE(couchbase_replace, NULL)
    PHP_FE(couchbase_remove, NULL)
    PHP_FE(couchbase_mtouch, NULL)
    PHP_FE(couchbase_arithmetic, NULL)

    // callbacks
    PHP_FE(couchbase_set_storage_callback, NULL)
    PHP_FE(couchbase_set_get_callback, NULL)
    PHP_FE(couchbase_set_remove_callback, NULL)
    PHP_FE(couchbase_set_touch_callback, NULL)
    PHP_FE(couchbase_set_arithmetic_callback, NULL)
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

enum php_libcouchbase_error_t {
    COUCHBASE_SUCCESS,
    COUCHBASE_AUTH_CONTINUE,
    COUCHBASE_AUTH_ERROR,
    COUCHBASE_DELTA_BADVAL,
    COUCHBASE_E2BIG,
    COUCHBASE_EBUSY,
    COUCHBASE_EINTERNAL,
    COUCHBASE_EINVAL,
    COUCHBASE_ENOMEM,
    COUCHBASE_ERANGE,
    COUCHBASE_ERROR,
    COUCHBASE_ETMPFAIL,
    COUCHBASE_KEY_EEXISTS,
    COUCHBASE_KEY_ENOENT,
    COUCHBASE_LIBEVENT_ERROR,
    COUCHBASE_NETWORK_ERROR,
    COUCHBASE_NOT_MY_VBUCKET,
    COUCHBASE_NOT_STORED,
    COUCHBASE_NOT_SUPPORTED,
    COUCHBASE_UNKNOWN_COMMAND,
    COUCHBASE_UNKNOWN_HOST,
};

// php boilerplate

static void php_couchbase_instance_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
    php_couchbase_instance *php_instance = (php_couchbase_instance*)rsrc->ptr;
    if (php_instance) {
        if (php_instance->instance) {
	    php_couchbase_callbacks *callbacks = (php_couchbase_callbacks *)libcouchbase_get_cookie(php_instance->instance);
	    if(callbacks) {
		if(callbacks->storage != NULL) {
		    zval_dtor(callbacks->storage);
		}
		if(callbacks->get != NULL) {
		    zval_dtor(callbacks->get);
		}
		if(callbacks->remove != NULL) {
		    zval_dtor(callbacks->remove);
		}
		if(callbacks->touch != NULL) {
		    zval_dtor(callbacks->touch);
		}
		if(callbacks->arithmetic != NULL) {
		    zval_dtor(callbacks->arithmetic);
		}
		efree(callbacks);
	    }
            libcouchbase_destroy(php_instance->instance);
        }
        if (php_instance->evbase) {
            event_base_free(php_instance->evbase);
        }
        efree(php_instance);
    }
}

PHP_MINIT_FUNCTION(couchbase)
{
    le_couchbase_instance = zend_register_list_destructors_ex(php_couchbase_instance_dtor, NULL, PHP_COUCHBASE_INSTANCE, module_number);

    // register constants
    // TODO, make PHP-land constants lose the LIB prefix
    REGISTER_LONG_CONSTANT("COUCHBASE_SUCCESS", LIBCOUCHBASE_SUCCESS, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_CONTINUE", LIBCOUCHBASE_AUTH_CONTINUE, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_ERROR", LIBCOUCHBASE_AUTH_ERROR, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_DELTA_BADVAL", LIBCOUCHBASE_DELTA_BADVAL, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_E2BIG", LIBCOUCHBASE_E2BIG, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_EBUSY", LIBCOUCHBASE_EBUSY, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINTERNAL", LIBCOUCHBASE_EINTERNAL, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINVAL", LIBCOUCHBASE_EINVAL, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_ENOMEM", LIBCOUCHBASE_ENOMEM, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERANGE", LIBCOUCHBASE_ERANGE, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERROR", LIBCOUCHBASE_ERROR, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_ETMPFAIL", LIBCOUCHBASE_ETMPFAIL, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_EEXISTS", LIBCOUCHBASE_KEY_EEXISTS, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_ENOENT", LIBCOUCHBASE_KEY_ENOENT, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_LIBEVENT_ERROR", LIBCOUCHBASE_LIBEVENT_ERROR, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_NETWORK_ERROR", LIBCOUCHBASE_NETWORK_ERROR, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_MY_VBUCKET", LIBCOUCHBASE_NOT_MY_VBUCKET, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_STORED", LIBCOUCHBASE_NOT_STORED, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_SUPPORTED", LIBCOUCHBASE_NOT_SUPPORTED, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_COMMAND", LIBCOUCHBASE_UNKNOWN_COMMAND, CONST_PERSISTENT);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_HOST", LIBCOUCHBASE_UNKNOWN_HOST, CONST_PERSISTENT);

    return SUCCESS;
}

long map_error_constant(libcouchbase_error_t error)
{
    switch(error) {
	    case LIBCOUCHBASE_SUCCESS: return COUCHBASE_SUCCESS;
	    case LIBCOUCHBASE_AUTH_CONTINUE: return COUCHBASE_AUTH_CONTINUE;
	    case LIBCOUCHBASE_AUTH_ERROR: return COUCHBASE_AUTH_ERROR;
	    case LIBCOUCHBASE_DELTA_BADVAL: return COUCHBASE_DELTA_BADVAL;
	    case LIBCOUCHBASE_E2BIG: return COUCHBASE_E2BIG;
	    case LIBCOUCHBASE_EBUSY: return COUCHBASE_EBUSY;
	    case LIBCOUCHBASE_EINTERNAL: return COUCHBASE_EINTERNAL;
	    case LIBCOUCHBASE_EINVAL: return COUCHBASE_EINVAL;
	    case LIBCOUCHBASE_ENOMEM: return COUCHBASE_ENOMEM;
	    case LIBCOUCHBASE_ERANGE: return COUCHBASE_ERANGE;
	    case LIBCOUCHBASE_ERROR: return COUCHBASE_ERROR;
	    case LIBCOUCHBASE_ETMPFAIL: return COUCHBASE_ETMPFAIL;
	    case LIBCOUCHBASE_KEY_EEXISTS: return COUCHBASE_KEY_EEXISTS;
	    case LIBCOUCHBASE_KEY_ENOENT: return COUCHBASE_KEY_ENOENT;
	    case LIBCOUCHBASE_LIBEVENT_ERROR: return COUCHBASE_LIBEVENT_ERROR;
	    case LIBCOUCHBASE_NETWORK_ERROR: return COUCHBASE_NETWORK_ERROR;
	    case LIBCOUCHBASE_NOT_MY_VBUCKET: return COUCHBASE_NOT_MY_VBUCKET;
	    case LIBCOUCHBASE_NOT_STORED: return COUCHBASE_NOT_STORED;
	    case LIBCOUCHBASE_NOT_SUPPORTED: return COUCHBASE_NOT_SUPPORTED;
	    case LIBCOUCHBASE_UNKNOWN_COMMAND: return COUCHBASE_UNKNOWN_COMMAND;
	    case LIBCOUCHBASE_UNKNOWN_HOST: return COUCHBASE_UNKNOWN_HOST;
    }
}

// functions
#include "functions/version.c"
#include "functions/create.c"
#include "functions/execute.c"
#include "functions/mget.c"
#include "functions/storage.c"
#include "functions/mtouch.c"
#include "functions/remove.c"
#include "functions/set_callback.c"
#include "functions/arithmetic.c"

// callbacks
#include "callbacks/touch.c"
#include "callbacks/remove.c"
#include "callbacks/storage.c"
#include "callbacks/get.c"
#include "callbacks/arithmetic.c"
