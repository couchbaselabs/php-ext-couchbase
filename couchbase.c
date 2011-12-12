/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2011 Couchbase, Inc.                                       |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License.     |
  | You may obtain a copy of the License at                              |
  |     http://www.apache.org/licenses/LICENSE-2.0                       |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
  | Author: Xinchen Hui    <laruence@php.net>                            |
  +----------------------------------------------------------------------+
*/

/* $Id */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "libcouchbase/couchbase.h"
#include "php_couchbase.h"

ZEND_DECLARE_MODULE_GLOBALS(couchbase)

static int le_couchbase;

/** {{{ COUCHBASE_ARG_INFO
 */
COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_connect, 0, 0, 1)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, user)
    ZEND_ARG_INFO(0, password)
    ZEND_ARG_INFO(0, bucket)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_add, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_set, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_cas, 0, 0, 4)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, cas)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, cache_cb)
    ZEND_ARG_INFO(1, cas_token)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_multi, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_ARRAY_INFO(0, keys, 0)
    ZEND_ARG_ARRAY_INFO(1, cas_tokens, 1)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_increment, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_decrement, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_delete, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_flush, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_last_error, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_version, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ couchbase_functions[]
 */
static zend_function_entry couchbase_functions[] = {
    PHP_FE(couchbase_connect, arginfo_connect)
    PHP_FE(couchbase_add, arginfo_add)
    PHP_FE(couchbase_set, arginfo_add)
    PHP_FE(couchbase_cas, arginfo_cas)
    PHP_FE(couchbase_get, arginfo_get)
    PHP_FE(couchbase_get_multi, arginfo_get_multi)
    PHP_FE(couchbase_increment, arginfo_increment)
    PHP_FE(couchbase_decrement, arginfo_decrement)
    PHP_FE(couchbase_delete, arginfo_delete)
    PHP_FE(couchbase_flush, arginfo_flush)
    PHP_FE(couchbase_last_error, arginfo_last_error)
    PHP_FE(couchbase_version, arginfo_version)
    /*
    PHP_FE(couchbase_execute, NULL)
    PHP_FE(couchbase_mget, NULL)
    PHP_FE(couchbase_set, NULL)
    PHP_FE(couchbase_replace, NULL)
    PHP_FE(couchbase_remove, NULL)
    PHP_FE(couchbase_mtouch, NULL)
    PHP_FE(couchbase_arithmetic, NULL)
    */
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ couchbase_module_entry
 */
zend_module_entry couchbase_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    "couchbase",
    couchbase_functions,
    PHP_MINIT(couchbase),
    PHP_MSHUTDOWN(couchbase),
    PHP_RINIT(couchbase),        /* Replace with NULL if there's nothing to do at request start */
    PHP_RSHUTDOWN(couchbase),    /* Replace with NULL if there's nothing to do at request end */
    PHP_MINFO(couchbase),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_COUCHBASE_VERSION,
#endif
    PHP_MODULE_GLOBALS(couchbase),
    PHP_GINIT(couchbase),
    NULL,
    NULL,
    STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#ifdef COMPILE_DL_COUCHBASE
ZEND_GET_MODULE(couchbase)
#endif

/* {{{ OnUpdateCompressionType
 */
static PHP_INI_MH(OnUpdateCompressionType) {
    if (!new_value) {
        COUCHBASE_G(compression_type) = COMPRESSION_TYPE_FASTLZ;
    } else if (!strcmp(new_value, "fastlz")) {
        COUCHBASE_G(compression_type) = COMPRESSION_TYPE_FASTLZ;
    } else if (!strcmp(new_value, "zlib")) {
        COUCHBASE_G(compression_type) = COMPRESSION_TYPE_ZLIB;
    } else {
        return FAILURE;
    }
    return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
}
/* }}} */

/* {{{ OnUpdateSerializer
 */
static PHP_INI_MH(OnUpdateSerializer) {
    if (!new_value) {
        COUCHBASE_G(serializer) = COUCHBASE_SERIALIZER_DEFAULT;
    } else if (!strcmp(new_value, "php")) {
        COUCHBASE_G(serializer) = COUCHBASE_SERIALIZER_PHP;
#ifdef HAVE_JSON_API
    } else if (!strcmp(new_value, "json")) {
        COUCHBASE_G(serializer) = COUCHBASE_SERIALIZER_JSON;
    } else if (!strcmp(new_value, "json_array")) {
        COUCHBASE_G(serializer) = COUCHBASE_SERIALIZER_JSON_ARRAY;
#endif
    } else {
        return FAILURE;
    }

    return OnUpdateString(entry, new_value, new_value_length, mh_arg1, mh_arg2, mh_arg3, stage TSRMLS_CC);
}
/* }}} */

/* {{{ PHP_INI
 */
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("couchbase.serializer", "php", PHP_INI_ALL, OnUpdateSerializer, serializer, zend_couchbase_globals, couchbase_globals)
    STD_PHP_INI_ENTRY("couchbase.compression_type", "fastlz", PHP_INI_ALL, OnUpdateCompressionType, compression_type, zend_couchbase_globals, couchbase_globals)
PHP_INI_END()
/* }}} */

static void php_couchbase_error_callback(libcouchbase_t handle, libcouchbase_error_t error, const char *errinfo) /* {{{ */ {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)libcouchbase_get_cookie(handle);
    /**
     * @FIXME: when connect to a non-couchbase-server port (but the socket is valid)
     * like a apache server, process will be hanged by event_loop
     */
    ctx->res->io->stop_event_loop(ctx->res->io);
}
/* }}} */

/* {{{ static void php_couchbase_get_callback(...)
 */
static void
php_couchbase_get_callback(libcouchbase_t handle,
                            const void *cookie,
                            libcouchbase_error_t error,
                            const void *key, size_t nkey,
                            const void *bytes, size_t nbytes,
                            uint32_t flag, uint64_t cas) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->exception = error;

    if (LIBCOUCHBASE_SUCCESS != error && LIBCOUCHBASE_KEY_ENOENT != error) {
        ctx->res->io->stop_event_loop(ctx->res->io);
        return;
    }

    if (LIBCOUCHBASE_KEY_ENOENT == error) {
        ZVAL_NULL(ctx->rv);
        return;
    }

    if (IS_ARRAY == Z_TYPE_P(ctx->rv)) { /* multi get */
        zval *v;
        MAKE_STD_ZVAL(v);
        ZVAL_STRINGL(v, (char *)bytes, nbytes, 1);
        zend_hash_add(Z_ARRVAL_P(ctx->rv), (char *)key, nkey + 1, (void **)&v, sizeof(zval *), NULL);
        if (ctx->cas) {
            zval *c;
            MAKE_STD_ZVAL(c);
            ZVAL_DOUBLE(c, cas);
            zend_hash_add(Z_ARRVAL_P(ctx->cas), (char *)key, nkey + 1, (void **)&c, sizeof(zval *), NULL);
        }
    } else {
        ZVAL_STRINGL(ctx->rv, (char *)bytes, nbytes, 1);
        if (ctx->cas) {
            ZVAL_DOUBLE(ctx->cas, cas);
        }
    }
}
/* }}} */

/* {{{ static void php_couchbase_storage_callback(...)
 */
static void
php_couchbase_storage_callback(libcouchbase_t handle,
                                const void *cookie,
                                libcouchbase_storage_t operation,
                                libcouchbase_error_t error,
                                const void *key, size_t nkey,
                                uint64_t cas) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);
    php_ignore_value(operation);
    php_ignore_value(key);
    php_ignore_value(nkey);
    php_ignore_value(cas);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->exception = error;
    if (error != LIBCOUCHBASE_SUCCESS && error != LIBCOUCHBASE_AUTH_CONTINUE) {
        return;
    }

    ZVAL_DOUBLE(ctx->rv, cas);
}
/* }}} */

/* {{{ static void php_couchbase_remove_callback(...) */
static void
php_couchbase_remove_callback(libcouchbase_t handle,
                            const void *cookie,
                            libcouchbase_error_t error,
                            const void *key, size_t nkey) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);
    php_ignore_value(key);
    php_ignore_value(nkey);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->exception = error;
}
/* }}} */

/* {{{ static void php_couchbase_flush_callback(...) */
static void
php_couchbase_flush_callback(libcouchbase_t handle,
                            const void* cookie,
                            const char* authority,
                            libcouchbase_error_t error) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);
    php_ignore_value(authority);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->exception = error;
}
/* }}} */

/* {{{ static void php_couchbase_arithmetic_callback(...) */
static void
php_couchbase_arithmetic_callback(libcouchbase_t handle,
                                const void *cookie,
                                libcouchbase_error_t error,
                                const void *key, size_t nkey,
                                uint64_t value, uint64_t cas) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);
    php_ignore_value(key);
    php_ignore_value(nkey);
    php_ignore_value(cas);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->exception = error;
    if (LIBCOUCHBASE_SUCCESS != error) {
        return;
    }

    ZVAL_LONG(ctx->rv, value);
}
/* }}} */

static void php_couchbase_res_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */ {
    php_couchbase_res *couchbase_res = (php_couchbase_res*)rsrc->ptr;
    if (couchbase_res) {
        if (couchbase_res->handle) {
            libcouchbase_destroy(couchbase_res->handle);
        }
        efree(couchbase_res);
    }
}
/* }}} */

static void php_couchbase_get_impl(INTERNAL_FUNCTION_PARAMETERS TSRMLS_DC, int multi) /* {{{ */ {
    char *key, **keys;
    long *klens, klen = 0;
    int  nkey, flag = 0;
    zval *res, *callback = NULL, *cas_token = NULL;

    if (multi) {
        zval *akeys;
        zval **ppzval;
        int i;
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|zl", &res, &akeys, &cas_token, &flag) == FAILURE) {
            return;
        }

        if (cas_token && IS_ARRAY != Z_TYPE_P(cas_token)) {
            array_init(cas_token);
        }
        array_init(return_value);

        nkey = zend_hash_num_elements(Z_ARRVAL_P(akeys));
        keys = emalloc(nkey * sizeof(char *));
        klens = emalloc(nkey * sizeof(long));

        for(i=0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
                zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
                zend_hash_move_forward(Z_ARRVAL_P(akeys)), i++) {
            if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void**)&ppzval) == FAILURE) {
                keys[i] = NULL;
                klens[i] = 0;
                continue;
            }

            if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
                convert_to_string_ex(ppzval);
            }

            keys[i] = Z_STRVAL_PP(ppzval);
            klens[i] = Z_STRLEN_PP(ppzval);
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|zz", &res, &key, &klen, &callback, &cas_token) == FAILURE) {
            return;
        }
        nkey = 1;
        keys = &key;
        klens = &klen;
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv  = return_value;
        ctx->cas = cas_token;

        retval = libcouchbase_mget(couchbase_res->handle, (const void *)ctx, nkey, (const void *)keys, (size_t *)klens, NULL);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            if (multi) {
                efree(keys);
                efree(klens);
            }
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule get request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += nkey;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->exception && LIBCOUCHBASE_KEY_ENOENT != ctx->exception) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to get a value from server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
        }
        if (multi) {
            efree(keys);
            efree(klens);
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, libcouchbase_storage_t op) /* {{{ */ {
    char *key;
    time_t exp = {0};
    long  klen, vlen, expire = 0;
    zval *res, *value;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz|l", &res, &key, &klen, &value, &expire) == FAILURE) {
        return;
    } else {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        if (Z_TYPE_P(value) != IS_STRING) {
            convert_to_string_ex(&value);
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv  = return_value;

        if (expire) {
            exp = expire;
        }

        retval = libcouchbase_store(couchbase_res->handle,
                (const void *)ctx, op, key, klen, Z_STRVAL_P(value), Z_STRLEN_P(value), 0, exp, 0);

        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule set request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        ++couchbase_res->seqno;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (ctx->exception) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to store a value to server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
        }

        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAMETERS TSRMLS_DC) /* {{{ */ {
    char *key;
    zval *res;
    long klen = 0;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res, &key, &klen) == FAILURE) {
        return;
    } else {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;

        retval = libcouchbase_remove(couchbase_res->handle, (const void *)ctx, (const void *)key, klen, 0);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule delete request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->exception && LIBCOUCHBASE_KEY_ENOENT != ctx->exception) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to remove a value from server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
            RETURN_FALSE;
        }
        efree(ctx);
    }
    RETURN_TRUE;
}
/* }}} */

static void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS TSRMLS_DC) /* {{{ */ {
    zval *res;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
        return;
    } else {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;

        retval = libcouchbase_flush(couchbase_res->handle, (const void *)ctx);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule flush request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->exception) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to flush: %s", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
            RETURN_FALSE;
        }
        efree(ctx);
    }
    RETURN_TRUE;
}
/* }}} */

static void php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAMETERS, char op) /* {{{ */ {
    zval *res;
    char *key;
    long klen = 0, offset = 1;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|l", &res, &key, &klen, &offset) == FAILURE) {
        return;
    } else {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;
        long delta = (op == '+')? offset : -offset;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv = return_value;

        retval = libcouchbase_arithmetic(couchbase_res->handle, (const void *)ctx, (const void *)key, klen, delta, 0, 0, 0);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule rithmetic request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->exception) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to %s value in server: %s", (op == '+')? "increment" : "decrement", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
            RETURN_FALSE;
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAMETERS) /* {{{ */ {
    char *key;
    zval *res, *value;
    long klen, expire = 0;
    double cas = 0.0;
    time_t exp = {0};

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rdsz|l", &res, &cas, &key, &klen, &value, &expire) == FAILURE) {
        return;
    } else {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv = return_value;

        if (expire) {
            exp = expire;
        }

        retval = libcouchbase_store(couchbase_res->handle, (const void *)ctx,
                LIBCOUCHBASE_SET, key, klen, Z_STRVAL_P(value), Z_STRLEN_P(value), 0, exp, (uint64_t)cas);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule cas request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        ++couchbase_res->seqno;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS == ctx->exception) {
            ZVAL_TRUE(return_value);
        } else if (LIBCOUCHBASE_KEY_EEXISTS == ctx->exception) {
            ZVAL_FALSE(return_value);
        } else {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to store a value to server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->exception));
        }

        efree(ctx);
    }
}
/* }}} */

/* {{{ proto couchbase_connect(string $host[, string $user[, string $password[, string $bucket]]])
*/
PHP_FUNCTION(couchbase_connect) {
    char *host;
    char *user = NULL, *passwd = NULL, *bucket = NULL;
    int host_len, user_len, passwd_len, bucket_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sss",
                &host, &host_len, &user, &user_len, &passwd, &passwd_len, &bucket, &bucket_len) == FAILURE) {
        return;
    } else {
        libcouchbase_t handle;
        libcouchbase_error_t retval;
        libcouchbase_io_opt_t *iops;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        iops = libcouchbase_create_io_ops(LIBCOUCHBASE_IO_OPS_DEFAULT, NULL, NULL);
        if (!iops) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create IO instance");
            RETURN_FALSE;
        }

        if (!bucket) {
            bucket = "default";
        }

        handle = libcouchbase_create(host, user, passwd, bucket, iops);
        if (!handle) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create libcouchbase instance");
            RETURN_FALSE;
        }

        php_ignore_value(libcouchbase_set_error_callback(handle, php_couchbase_error_callback));

        if (LIBCOUCHBASE_SUCCESS != (retval = libcouchbase_connect(handle))) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to connect libcouchbase to server: %s", libcouchbase_strerror(handle, retval));
            RETURN_FALSE;
        }

        php_ignore_value(libcouchbase_set_get_callback(handle, php_couchbase_get_callback));
        php_ignore_value(libcouchbase_set_storage_callback(handle, php_couchbase_storage_callback));
        php_ignore_value(libcouchbase_set_remove_callback(handle, php_couchbase_remove_callback));
        php_ignore_value(libcouchbase_set_flush_callback(handle, php_couchbase_flush_callback));
        php_ignore_value(libcouchbase_set_arithmetic_callback(handle, php_couchbase_arithmetic_callback));

        couchbase_res = emalloc(sizeof(php_couchbase_res));
        couchbase_res->handle = handle;
        couchbase_res->seqno = 0;
        couchbase_res->io = iops;

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        libcouchbase_set_cookie(handle, (const void *)ctx);
        libcouchbase_wait(handle);

        if (LIBCOUCHBASE_SUCCESS != (retval = libcouchbase_get_last_error(handle))) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to connect libcouchbase to server: %s", libcouchbase_strerror(handle, retval));
            libcouchbase_destroy(handle);
            efree(couchbase_res);
            RETURN_FALSE;
        }

        ZEND_REGISTER_RESOURCE(return_value, couchbase_res, le_couchbase);
    }
}
/* }}} */

/* {{{ proto couchbase_get(resource $couchbase, string $key[, callback $cache_cb[, float &$cas_tokey]])
 */
PHP_FUNCTION(couchbase_get) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_multi(resource $couchbase, array $keys[, array &cas[, int $flag]])
 */
PHP_FUNCTION(couchbase_get_multi) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto couchbase_cas(resource $couchbase, float $cas, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_cas) {
    php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto couchbase_add(resource $couchbase, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_add) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_ADD);
}
/* }}} */

/* {{{ proto couchbase_set(resource $couchbase, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_set) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET);
}
/* }}} */

/* {{{ proto couchbase_increment(resource $couchbase, string $key[, int $offset = 1])
 */
PHP_FUNCTION(couchbase_increment) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '+');
}
/* }}} */

/* {{{ proto couchbase_decrement(resource $couchbase, string $key[, int $offset = 1])
 */
PHP_FUNCTION(couchbase_decrement) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '-');
}
/* }}} */

/* {{{ proto couchbase_delete(resource $couchbase, string $key)
 */
PHP_FUNCTION(couchbase_delete) {
    php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto couchbase_flush(resource $couchbase)
 */
PHP_FUNCTION(couchbase_flush) {
    php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto couchbase_last_error(resource $couchbase)
 */
PHP_FUNCTION(couchbase_last_error) {
    zval *res;
    php_couchbase_res *couchbase_res;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
        return;
    }
    ZEND_FETCH_RESOURCE(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase);
    RETURN_LONG(libcouchbase_get_last_error(couchbase_res->handle));
}
/* }}} */

/* {{{ proto couchbase_version(void)
 */
PHP_FUNCTION(couchbase_version) {
    RETURN_STRING(PHP_COUCHBASE_VERSION, 1);
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
PHP_GINIT_FUNCTION(couchbase) {
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(couchbase) {
    REGISTER_INI_ENTRIES();

    REGISTER_LONG_CONSTANT("COUCHBASE_SUCCESS",         LIBCOUCHBASE_SUCCESS, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_CONTINUE",     LIBCOUCHBASE_AUTH_CONTINUE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_ERROR",         LIBCOUCHBASE_AUTH_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_DELTA_BADVAL",     LIBCOUCHBASE_DELTA_BADVAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_E2BIG",             LIBCOUCHBASE_E2BIG, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EBUSY",             LIBCOUCHBASE_EBUSY, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINTERNAL",         LIBCOUCHBASE_EINTERNAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINVAL",             LIBCOUCHBASE_EINVAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ENOMEM",             LIBCOUCHBASE_ENOMEM, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERANGE",             LIBCOUCHBASE_ERANGE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERROR",             LIBCOUCHBASE_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ETMPFAIL",         LIBCOUCHBASE_ETMPFAIL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_EEXISTS",     LIBCOUCHBASE_KEY_EEXISTS, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_ENOENT",         LIBCOUCHBASE_KEY_ENOENT, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NETWORK_ERROR",     LIBCOUCHBASE_NETWORK_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_MY_VBUCKET",     LIBCOUCHBASE_NOT_MY_VBUCKET, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_STORED",         LIBCOUCHBASE_NOT_STORED, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_SUPPORTED",     LIBCOUCHBASE_NOT_SUPPORTED, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_COMMAND", LIBCOUCHBASE_UNKNOWN_COMMAND, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_HOST",     LIBCOUCHBASE_UNKNOWN_HOST, CONST_PERSISTENT | CONST_CS);

    le_couchbase = zend_register_list_destructors_ex(php_couchbase_res_dtor, NULL, PHP_COUCHBASE_RESOURCE, module_number);

    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(couchbase) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(couchbase) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(couchbase) {
    return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
*/
PHP_MINFO_FUNCTION(couchbase)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "couchbase support", "enabled");
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
