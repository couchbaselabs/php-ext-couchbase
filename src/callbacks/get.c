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
