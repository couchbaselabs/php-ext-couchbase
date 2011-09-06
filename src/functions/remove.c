
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

