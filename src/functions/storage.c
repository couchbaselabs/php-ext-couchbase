
PHP_FUNCTION(couchbase_set)
{
    couchbase_store(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET);
}

PHP_FUNCTION(couchbase_add)
{
    couchbase_store(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_ADD);
}

PHP_FUNCTION(couchbase_replace)
{
    couchbase_store(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_REPLACE);
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

