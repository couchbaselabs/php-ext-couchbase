PHP_FUNCTION(couchbase_set_storage_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, STORAGE_CALLBACK);
}

PHP_FUNCTION(couchbase_set_get_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, GET_CALLBACK);
}

PHP_FUNCTION(couchbase_set_remove_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, REMOVE_CALLBACK);
}

PHP_FUNCTION(couchbase_set_touch_callback)
{
    couchbase_set_callback(INTERNAL_FUNCTION_PARAM_PASSTHRU, TOUCH_CALLBACK);
}

static void couchbase_set_callback(INTERNAL_FUNCTION_PARAMETERS, php_couchbase_callback_type type)
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
    case STORAGE_CALLBACK:
	if(callbacks->storage != NULL) {
	    zval_dtor(callbacks->storage);
	}
	callbacks->storage = zcallback;
	break;
    case GET_CALLBACK:
	if(callbacks->get != NULL) {
	    zval_dtor(callbacks->get);
	}
	callbacks->get = zcallback;
	break;
    case REMOVE_CALLBACK:
	if(callbacks->remove != NULL) {
	    zval_dtor(callbacks->remove);
	}
	callbacks->remove = zcallback;
	break;
    case TOUCH_CALLBACK:
	if(callbacks->touch != NULL) {
	    zval_dtor(callbacks->touch);
	}
	callbacks->touch = zcallback;
	break;
    }

    libcouchbase_set_cookie(php_instance->instance, callbacks);
    RETURN_TRUE;
}
