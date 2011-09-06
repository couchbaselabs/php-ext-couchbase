PHP_FUNCTION(couchbase_mtouch)
{
    zval *zinstance;
    zval *result;
    zval *zexpiration;
    const char *key = NULL; int key_len;


    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz",
        &zinstance,
        &key, &key_len,
	&zexpiration
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

    error = libcouchbase_mtouch(php_instance->instance,
			      NULL,
                              1,
                              (const void * const *)keys,
                              keys_sizes,
			      (const time_t *)&Z_LVAL_P(zexpiration));
    if(error != LIBCOUCHBASE_SUCCESS) {
        ZVAL_LONG(return_value, map_error_constant(error));
    }

    efree(keys);
    efree(keys_sizes);
}
