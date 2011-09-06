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

