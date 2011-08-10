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
    PHP_FE(couchbase_execute, NULL)
    PHP_FE(couchbase_get, NULL)
    PHP_FE(couchbase_set, NULL)
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
    RETURN_TRUE;
}

static void get_callback(libcouchbase_t instance,
                         libcouchbase_error_t error,
                         const void *key, size_t nkey,
                         const void *bytes, size_t nbytes,
                         uint32_t flags, uint64_t cas)
{
    php_printf("> get_callback\n");
    zval *callback = (zval *)libcouchbase_get_cookie(instance);
	char *callback_name;
    if (Z_TYPE_P(callback) != IS_NULL) {
        if (!zend_is_callable(callback, 0, &callback_name)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "The callback, '%s', is invalid", callback_name);
            efree(callback_name);
        }
        efree(callback_name);
    }
    php_printf("callback ok\n");
    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    zval *result;
    zval **argv[2];
    zval *value = NULL;
    zval *error_msg = NULL;

    MAKE_STD_ZVAL(error_msg);
    if(error != LIBCOUCHBASE_SUCCESS) {
        php_printf("get callback no success\n");
        ZVAL_STRING(error_msg, "some error string", 1);
    } else {
        ZVAL_NULL(error_msg)
    }

    MAKE_STD_ZVAL(value);
    ZVAL_STRINGL(value, bytes, nbytes, 1);

    argv[0] = &error_msg;
    argv[1] = &value;

    php_debug_zval_dump(&value);

    php_printf("args ok\n");

    if (Z_TYPE_P(callback) != IS_NULL) {
        zend_fcall_info fci;

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = callback;
        fci.symbol_table = NULL;
        fci.retval_ptr_ptr = &result;
        fci.param_count = 2;
        fci.params = argv;
        fci.no_separation = 0;

        php_printf("fci ok\n");

        if (zend_call_function(&fci, &fci_cache TSRMLS_CC) != SUCCESS ) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the map callback");
        }
    }

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&callback);
    zval_ptr_dtor(argv[0]);
    zval_ptr_dtor(argv[1]);
    php_printf("< get_callback\n");
}

PHP_FUNCTION(couchbase_get)
{
    php_printf("> couchbase_get\n");

    zval *zinstance;
    zval *callback = NULL;
    zval *result, *null;

    const char *key = NULL; int key_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz",
        &zinstance,
        &key, &key_len,
        &callback
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    php_printf("resource\n");

    libcouchbase_error_t error;
    char** keys = ecalloc((size_t)(1), sizeof(char*));;
    size_t* keys_sizes = ecalloc((size_t)(1), sizeof(size_t));;
    keys[0] = (char *)key;
    keys_sizes[0] = key_len;
    zval *callback_retained;

    MAKE_STD_ZVAL(callback_retained);
    ZVAL_ZVAL(callback_retained, callback, 1, 1);

    (void)libcouchbase_set_cookie(php_instance->instance, callback_retained);
    php_printf("callback\n");
    (void)libcouchbase_set_get_callback(php_instance->instance, get_callback);
    php_printf("cookie\n");
    php_printf("callback\n");

    error = libcouchbase_mget(php_instance->instance,
                              1,
                              (const void * const *)keys,
                              keys_sizes,
                              NULL);
    if(error != LIBCOUCHBASE_SUCCESS) {
        php_printf("mget failed!\n");
    }
    php_printf("mget yay\n");

    efree(keys);
    efree(keys_sizes);

    php_printf("< couchbase_get\n");
    RETURN_TRUE;
}

void command_cookie_push(void *stack, void *user_data)
{
    if(stack == NULL) {
        // init stack
    }

    void *elm;
    elm->next = stack;
    elm->data = user_data;
}

void *command_cookie_pop(void *stack)
{
    if(stack == NULL) {
        return NULL;
    }

    void *elm = stack;
    stack = elm->next;
    return elm->data;
}

void set_command_cookie(libcouchbase_t instance, int cmd, void *user_data)
{
    void *cookie = (void *)libcouchbase_get_cookie(instance);
    switch(cmd) {
    case CMD_SET:
        command_cookie_push(cookie->set, user_data);
        break;
    }
    (void)libcouchbase_set_cookie(php_instance->instance, cookie);
}

void *get_command_cookie(libcouchbase_t instance, int cmd)
{
    void *cookie = (void *)libcouchbase_get_cookie(instance);
    void *user_data = NULL;
    switch(cmd) {
    case CMD_SET:
        user_data = (void *)command_cookie_pop(cookie->set);
        break;
    }
    (void)libcouchbase_set_cookie(php_instance->instance, cookie);
    return command_cookie_pop(instance, cmd);
}

static void storage_callback(libcouchbase_t instance,
                             libcouchbase_error_t error,
                             const void *key, size_t nkey,
                             uint64_t cas)
{

    zval *callback = (zval *) get_command_cookie(instance);
	char *callback_name;
    if (Z_TYPE_P(callback) != IS_NULL) {
        if (!zend_is_callable(callback, 0, &callback_name)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "The callback, '%s', is invalid", callback_name);
            efree(callback_name);
        }
        efree(callback_name);
    }

    zend_fcall_info_cache fci_cache = empty_fcall_info_cache;

    zval *result;
    zval **argv[1];
    zval *error_msg = NULL;

    MAKE_STD_ZVAL(error_msg);
    if(error != LIBCOUCHBASE_SUCCESS) {
        php_printf("get callback no success\n");
        ZVAL_STRING(error_msg, "some error string", 1);
    } else {
        ZVAL_NULL(error_msg)
    }

    argv[0] = &error_msg;

    if (Z_TYPE_P(callback) != IS_NULL) {
        zend_fcall_info fci;

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = callback;
        fci.symbol_table = NULL;
        fci.retval_ptr_ptr = &result;
        fci.param_count = 1;
        fci.params = argv;
        fci.no_separation = 0;

        if (zend_call_function(&fci, &fci_cache TSRMLS_CC) != SUCCESS ) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "An error occurred while invoking the map callback");
        }
    }

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&callback);
    zval_ptr_dtor(argv[0]);
}

PHP_FUNCTION(couchbase_set)
{
    zval *zinstance;
    zval *callback = NULL;
    zval *result, *value;

    const char *key = NULL; int key_len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rszz",
        &zinstance,
        &key, &key_len,
        &value,
        &callback
    ) == FAILURE) {
        RETURN_FALSE;
    }

    php_couchbase_instance *php_instance;
    ZEND_FETCH_RESOURCE(php_instance, php_couchbase_instance*,
        &zinstance, -1, PHP_COUCHBASE_VERSION, le_couchbase_instance);

    libcouchbase_error_t error;
    void *bytes = estrdup(Z_STRVAL_P(value));
    size_t nbytes = strlen(bytes);

    zval *callback_retained;
    MAKE_STD_ZVAL(callback_retained);
    ZVAL_ZVAL(callback_retained, callback, 1, 1);

    set_command_cookie(php_instance->instance, CMD_SET, callback_retained);
    (void)libcouchbase_set_storage_callback(php_instance->instance, storage_callback);

    error = libcouchbase_store(php_instance->instance,
                               LIBCOUCHBASE_SET,
                               (const void * const *)key, (size_t)strlen(key),
                               bytes, nbytes,
                               0, 0, 0);
    if(error != LIBCOUCHBASE_SUCCESS) {
        php_printf("set failed!\n");
    }

    efree(bytes);
    RETURN_TRUE;
}
#ifdef __cplusplus
}
#endif
