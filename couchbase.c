#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

//#ifdef __cplusplus
//extern "C" {
//#endif

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
    PHP_FE(couchbase_remove, NULL)
    PHP_FE(couchbase_set_storage_callback, NULL)
    PHP_FE(couchbase_set_get_callback, NULL)
    PHP_FE(couchbase_set_remove_callback, NULL)
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

// forward declaration
long map_error_constant(libcouchbase_error_t error);
static void couchbase_store(INTERNAL_FUNCTION_PARAMETERS, libcouchbase_storage_t operation);
static void couchbase_set_callback(INTERNAL_FUNCTION_PARAMETERS, int type);
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
void remove_callback(libcouchbase_t instance,
                     const void *cookie,
                     libcouchbase_error_t error,
                     const void *key, size_t nkey);

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
    struct event_base *evbase = event_base_new();
    if (evbase == NULL) {
        php_printf("Failed to create event base for libcouchbase\n");
        RETURN_FALSE;
    }

    libcouchbase_t instance = libcouchbase_create(host, user,
                                                  passwd, bucket, evbase);
    if (instance == NULL) {
        event_base_free(evbase);
        php_printf("Failed to create libcouchbase instance\n");
        RETURN_FALSE;
    }

    if (libcouchbase_connect(instance) != LIBCOUCHBASE_SUCCESS) {
        event_base_free(evbase);
        php_printf("Failed to connect libcouchbase instance to server\n");
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    php_instance = emalloc(sizeof(php_couchbase_instance));
    php_instance->instance = instance;
    php_instance->evbase = evbase;

    ZEND_REGISTER_RESOURCE(return_value, php_instance, le_couchbase_instance);

    // initialize callbacks
    php_couchbase_callbacks *callbacks = emalloc(sizeof(php_couchbase_callbacks));
    callbacks->storage = NULL;
    callbacks->get = NULL;
    libcouchbase_set_cookie(instance, callbacks);
    libcouchbase_set_storage_callback(instance, storage_callback);
    libcouchbase_set_get_callback(instance, get_callback);
    libcouchbase_set_remove_callback(instance, remove_callback);
}

PHP_FUNCTION(couchbase_execute)
{
    zval *zinstance;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r",
        &zinstance
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    libcouchbase_execute(php_instance->instance);
}

PHP_FUNCTION(couchbase_set_storage_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}

PHP_FUNCTION(couchbase_set_get_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, 2);
}

PHP_FUNCTION(couchbase_set_remove_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, 3);
}

static void couchbase_set_callback(INTERNAL_FUNCTION_PARAMETERS, int type)
{
    zval *zinstance;
    zval *zcallback;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rz",
        &zinstance,
	&zcallback
    ) == FAILURE) {
	RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
	&zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    Z_ADDREF_P(zcallback);
    php_couchbase_callbacks *callbacks = (php_couchbase_callbacks *)libcouchbase_get_cookie(php_instance->instance);
    switch(type) {
    case 1:
	callbacks->storage = zcallback;
	break;
    case 2:
	callbacks->get = zcallback;
	break;
    case 3:
	callbacks->remove = zcallback;
	break;
    }

    libcouchbase_set_cookie(php_instance->instance, callbacks);

    RETURN_TRUE;
}

static void get_callback(libcouchbase_t instance,
			 const void *cookie,
                         libcouchbase_error_t error,
                         const void *key, size_t nkey,
                         const void *bytes, size_t nbytes,
                         uint32_t flags, uint64_t cas)
{
    php_couchbase_callbacks *callbacks = (php_couchbase_callbacks *)libcouchbase_get_cookie(instance);
    zval *zcallback = callbacks->get;
    char *callback_name;
    if (Z_TYPE_P(zcallback) != IS_NULL) {
        if (!zend_is_callable(zcallback, 0, &callback_name)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "The callback, '%s', is invalid", callback_name);
            efree(callback_name);
        }
        efree(callback_name);
    }

    zval *zresult;
    zval **zzargv[3];
    zval *zkey = NULL;
    zval *zvalue = NULL;
    zval *zerror = NULL;

    MAKE_STD_ZVAL(zerror);
    if(error != LIBCOUCHBASE_SUCCESS) {
        ZVAL_LONG(zerror, error);
    } else {
        ZVAL_NULL(zerror)
    }

    MAKE_STD_ZVAL(zkey);
    ZVAL_STRINGL(zkey, key, nkey, 1);

    MAKE_STD_ZVAL(zvalue);
    if(error == LIBCOUCHBASE_KEY_ENOENT) {
	ZVAL_NULL(zvalue);
    } else {
	ZVAL_STRINGL(zvalue, bytes, nbytes, 1);
    }

    zzargv[0] = &zerror;
    zzargv[1] = &zkey;
    zzargv[2] = &zvalue;

    if (Z_TYPE_P(zcallback) != IS_NULL) {
        zend_fcall_info fci;

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = zcallback;
        fci.symbol_table = NULL;
        fci.retval_ptr_ptr = &zresult;
        fci.param_count = 3;
        fci.params = zzargv;
        fci.no_separation = 0;
	fci.object_ptr = NULL;

        if (zend_call_function(&fci, NULL TSRMLS_CC) != SUCCESS ) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the get callback");
        }
    }

    zval_ptr_dtor(&zresult);
    zval_ptr_dtor(zzargv[0]);
    zval_ptr_dtor(zzargv[1]);
    zval_ptr_dtor(zzargv[2]);
}

PHP_FUNCTION(couchbase_mget)
{
    zval *zinstance;
    zval *result, *null;
    const char *key = NULL; int key_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
        &zinstance,
        &key, &key_len
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    libcouchbase_error_t error;
    char** keys = ecalloc((size_t)(1), sizeof(char*));;
    size_t* keys_sizes = ecalloc((size_t)(1), sizeof(size_t));;
    keys[0] = (char *)key;
    keys_sizes[0] = key_len;

    error = libcouchbase_mget(php_instance->instance,
			      NULL,
                              1,
                              (const void * const *)keys,
                              keys_sizes,
                              NULL);
    if(error != LIBCOUCHBASE_SUCCESS) {
        ZVAL_LONG(return_value, map_error_constant(error));
    }

    efree(keys);
    efree(keys_sizes);
}

static void storage_callback(libcouchbase_t instance,
			     const void *cookie,
			     libcouchbase_storage_t operation,
                             libcouchbase_error_t error,
                             const void *key, size_t nkey,
                             uint64_t cas)
{
    php_couchbase_callbacks *callbacks = (php_couchbase_callbacks *)libcouchbase_get_cookie(instance);
    zval *zcallback = callbacks->storage;

    char *callback_name;
    if (Z_TYPE_P(zcallback) != IS_NULL) {
        if (!zend_is_callable(zcallback, 0, &callback_name)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "The callback, '%s', is invalid", callback_name);
            efree(callback_name);
        }
        efree(callback_name);
    } // what if callback is null? bail!

    zval *result;
    zval **argv[2];
    zval *zerror = NULL;
    zval *zkey = NULL;

    MAKE_STD_ZVAL(zerror);
    if(error != LIBCOUCHBASE_SUCCESS) {
	ZVAL_LONG(zerror, error);
    } else {
	ZVAL_NULL(zerror);
    }

    MAKE_STD_ZVAL(zkey);
    ZVAL_STRINGL(zkey, key, nkey, 1);

    argv[0] = &zerror;
    argv[1] = &zkey;

    if (Z_TYPE_P(zcallback) != IS_NULL) {
        zend_fcall_info fci;

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = zcallback;
        fci.symbol_table = NULL;
        fci.retval_ptr_ptr = &result;
        fci.param_count = 2;
        fci.params = argv;
        fci.no_separation = 0;
        fci.object_ptr = NULL;

        if (zend_call_function(&fci, NULL TSRMLS_CC) != SUCCESS ) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the storage callback");
        }
    }
    zval_ptr_dtor(&result);
    zval_ptr_dtor(argv[0]);
    zval_ptr_dtor(argv[1]);
}

PHP_FUNCTION(couchbase_set)
{
    couchbase_store(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET);
}

PHP_FUNCTION(couchbase_add)
{
    couchbase_store(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_ADD);
}

static void couchbase_store(INTERNAL_FUNCTION_PARAMETERS, libcouchbase_storage_t operation)
{
    zval *zinstance;
    zval *value;

    const char *key = NULL; int key_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz",
        &zinstance,
        &key, &key_len,
        &value
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    libcouchbase_error_t error;
    void *bytes = estrdup(Z_STRVAL_P(value));
    size_t nbytes = strlen(bytes);

    error = libcouchbase_store(php_instance->instance,
			       NULL,
                               operation,
                               (const void * const *)key, (size_t)strlen(key),
                               bytes, nbytes,
                               0, 0, 0);
    if(error != LIBCOUCHBASE_SUCCESS) {
        ZVAL_LONG(return_value, map_error_constant(error));
    }

    efree(bytes);
    RETURN_TRUE;
}

void remove_callback(libcouchbase_t instance,
                     const void *cookie,
                     libcouchbase_error_t error,
                     const void *key, size_t nkey)
{
    php_couchbase_callbacks *callbacks = (php_couchbase_callbacks *)libcouchbase_get_cookie(instance);
    zval *zcallback = callbacks->remove;
    char *callback_name;

    if (Z_TYPE_P(zcallback) != IS_NULL) {
        if (!zend_is_callable(zcallback, 0, &callback_name)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "The callback, '%s', is invalid", callback_name);
            efree(callback_name);
        }
        efree(callback_name);
    }

    zval *result;
    zval **argv[2];
    zval *zerror = NULL;
    zval *zkey = NULL;

    MAKE_STD_ZVAL(zerror);
    if(error != LIBCOUCHBASE_SUCCESS) {
      ZVAL_LONG(zerror, error);
    } else {
      ZVAL_NULL(zerror);
    }

    MAKE_STD_ZVAL(zkey);
    ZVAL_STRINGL(zkey, key, nkey, 1);

    argv[0] = &zerror;
    argv[1] = &zkey;

    if (Z_TYPE_P(zcallback) != IS_NULL) {
        zend_fcall_info fci;

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = zcallback;
        fci.symbol_table = NULL;
        fci.retval_ptr_ptr = &result;
        fci.param_count = 2;
        fci.params = argv;
        fci.no_separation = 0;
	fci.object_ptr = NULL;

        if (zend_call_function(&fci, NULL TSRMLS_CC) != SUCCESS ) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the storage callback");
        }
    }

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&zerror);
    zval_ptr_dtor(&zcallback);
    zval_ptr_dtor(argv[0]);
    zval_ptr_dtor(argv[1]);
}

PHP_FUNCTION(couchbase_remove)
{
    zval *zinstance;

    const char *key = NULL; int key_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
        &zinstance,
        &key, &key_len
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    libcouchbase_error_t error;

    error = libcouchbase_remove(php_instance->instance,
				NULL,
                               (const void * const *)key, (size_t)strlen(key),
                               0);
    if(error != LIBCOUCHBASE_SUCCESS) {
        ZVAL_LONG(return_value, map_error_constant(error));
    }

    RETURN_TRUE;
}

//#ifdef __cplusplus
//}
//#endif
