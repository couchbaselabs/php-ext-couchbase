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
