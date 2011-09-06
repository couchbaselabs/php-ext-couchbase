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
    callbacks->remove = NULL;
    callbacks->touch = NULL;
    libcouchbase_set_cookie(instance, callbacks);

    libcouchbase_set_storage_callback(instance, storage_callback);
    libcouchbase_set_get_callback(instance, get_callback);
    libcouchbase_set_remove_callback(instance, remove_callback);
    libcouchbase_set_touch_callback(instance, touch_callback);
}
