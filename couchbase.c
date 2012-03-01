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
#include "ext/standard/url.h"
#include "ext/standard/php_smart_str.h"
#ifdef HAVE_JSON_API
# include "ext/json/php_json.h"
#endif
#include "ext/standard/php_var.h"
#include "libcouchbase/couchbase.h"
#include "php_couchbase.h"
#ifdef HAVE_COMPRESSION_FASTLZ
# include "fastlz.c"
#endif
#ifdef HAVE_COMPRESSION_ZLIB
# include "zlib.h"
#endif

ZEND_DECLARE_MODULE_GLOBALS(couchbase)

static int le_couchbase;
static int le_pcouchbase;
zend_class_entry *couchbase_ce;

/* {{{ COUCHBASE_FUNCTIONS_ARG_INFO
 */
COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_connect, 0, 0, 1)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, user)
    ZEND_ARG_INFO(0, password)
    ZEND_ARG_INFO(0, bucket)
    ZEND_ARG_INFO(0, persistent)
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
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_set_multi, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_ARRAY_INFO(0, values, 0)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_replace, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_append, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_prepend, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_delayed, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_ARRAY_INFO(0, keys, 0)
    ZEND_ARG_INFO(0, with_cas)
    ZEND_ARG_INFO(0, cb)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_fetch, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_fetch_all, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_increment, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, create)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, initial)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_decrement, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, create)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, initial)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_delete, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_stats, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_flush, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_result_code, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_set_option, 0, 0, 3)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, option)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_option, 0, 0, 2)
    ZEND_ARG_INFO(0, resource)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_version, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ COUCHBASE_METHODS_ARG_INFO
 */
COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 1)
    ZEND_ARG_INFO(0, host)
    ZEND_ARG_INFO(0, user)
    ZEND_ARG_INFO(0, password)
    ZEND_ARG_INFO(0, bucket)
    ZEND_ARG_INFO(0, persistent)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_add, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_set, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_setmulti, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, values, 0)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_replace, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_append, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_prepend, 0, 0, 2)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_cas, 0, 0, 3)
    ZEND_ARG_INFO(0, cas)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, expiration)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_get, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, cache_cb)
    ZEND_ARG_INFO(1, cas_token)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getmulti, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, keys, 0)
    ZEND_ARG_ARRAY_INFO(1, cas_tokens, 1)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getdelayed, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, keys, 0)
    ZEND_ARG_INFO(0, with_cas)
    ZEND_ARG_INFO(0, cb)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_fetch, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_fetchall, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_increment, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, create)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, initial)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_decrement, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, offset)
    ZEND_ARG_INFO(0, create)
    ZEND_ARG_INFO(0, expiration)
    ZEND_ARG_INFO(0, initial)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_delete, 0, 0, 1)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, cas)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getstats, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_flush, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_resultcode, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_setoption, 0, 0, 2)
    ZEND_ARG_INFO(0, option)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getoption, 0, 0, 1)
    ZEND_ARG_INFO(0, option)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ couchbase_functions[]
 */
static zend_function_entry couchbase_functions[] = {
    PHP_FE(couchbase_connect, arginfo_connect)
    PHP_FE(couchbase_add, arginfo_add)
    PHP_FE(couchbase_set, arginfo_set)
    PHP_FE(couchbase_set_multi, arginfo_set_multi)
    PHP_FE(couchbase_replace, arginfo_replace)
    PHP_FE(couchbase_prepend, arginfo_prepend)
    PHP_FE(couchbase_append, arginfo_append)
    PHP_FE(couchbase_cas, arginfo_cas)
    PHP_FE(couchbase_get, arginfo_get)
    PHP_FE(couchbase_get_multi, arginfo_get_multi)
    PHP_FE(couchbase_get_delayed, arginfo_get_delayed)
    PHP_FE(couchbase_fetch, arginfo_fetch)
    PHP_FE(couchbase_fetch_all, arginfo_fetch_all)
    PHP_FE(couchbase_increment, arginfo_increment)
    PHP_FE(couchbase_decrement, arginfo_decrement)
    PHP_FE(couchbase_get_stats, arginfo_get_stats)
    PHP_FE(couchbase_delete, arginfo_delete)
    PHP_FE(couchbase_flush, arginfo_flush)
    PHP_FE(couchbase_get_result_code, arginfo_result_code)
    PHP_FE(couchbase_set_option, arginfo_set_option)
    PHP_FE(couchbase_get_option, arginfo_get_option)
    PHP_FE(couchbase_get_version, arginfo_get_version)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ couchbase_methods[]
 */
static zend_function_entry couchbase_methods[] = {
    PHP_ME(couchbase, __construct, arginfo_construct, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
    PHP_ME(couchbase, add, arginfo_m_add, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, set, arginfo_m_set, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, setMulti, arginfo_m_setmulti, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, replace, arginfo_m_replace, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, prepend, arginfo_m_prepend, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, append, arginfo_m_append, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, cas, arginfo_m_cas, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, get, arginfo_m_get, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getMulti, arginfo_m_getmulti, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getDelayed, arginfo_m_getdelayed, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, fetch, arginfo_m_fetch, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, fetchAll, arginfo_m_fetchall, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, delete, arginfo_m_delete, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getStats, arginfo_m_getstats, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, flush, arginfo_m_flush, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, increment, arginfo_m_increment, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, decrement, arginfo_m_decrement, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getResultCode, arginfo_m_resultcode, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, setOption, arginfo_m_setoption, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getOption, arginfo_m_getoption, ZEND_ACC_PUBLIC)
    PHP_ME(couchbase, getVersion, arginfo_get_version, ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};
/* }}} */

/* {{{ couchbase_module_entry
 */
#if ZEND_MODULE_API_NO >= 20050922
static const zend_module_dep coucubase_deps[] = {
#ifdef HAVE_JSON_API
    ZEND_MOD_REQUIRED("json")
#endif
    {NULL, NULL, NULL}
};
#endif

zend_module_entry couchbase_module_entry = {
#if ZEND_MODULE_API_NO >= 20050922
    STANDARD_MODULE_HEADER_EX, NULL,
    (zend_module_dep*)coucubase_deps,
#else
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

/* {{{ OnUpdateCompressor
 */
static PHP_INI_MH(OnUpdateCompressor) {
    if (!new_value || !strcmp(new_value, "none")) {
        COUCHBASE_G(compressor_real) = COUCHBASE_COMPRESSION_NONE;
#ifdef HAVE_COMPRESSION_FASTLZ
    } else if (!strcmp(new_value, "fastlz")) {
        COUCHBASE_G(compressor_real) = COUCHBASE_COMPRESSION_FASTLZ;
#endif
#ifdef HAVE_COMPRESSION_ZLIB
    } else if (!strcmp(new_value, "zlib")) {
        COUCHBASE_G(compressor_real) = COUCHBASE_COMPRESSION_ZLIB;
#endif
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
        COUCHBASE_G(serializer_real) = COUCHBASE_SERIALIZER_DEFAULT;
    } else if (!strcmp(new_value, "php")) {
        COUCHBASE_G(serializer_real) = COUCHBASE_SERIALIZER_PHP;
#ifdef HAVE_JSON_API
    } else if (!strcmp(new_value, "json")) {
        COUCHBASE_G(serializer_real) = COUCHBASE_SERIALIZER_JSON;
    } else if (!strcmp(new_value, "json_array")) {
        COUCHBASE_G(serializer_real) = COUCHBASE_SERIALIZER_JSON_ARRAY;
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
    STD_PHP_INI_ENTRY("couchbase.compressor", "none",    PHP_INI_ALL, OnUpdateCompressor, compressor, zend_couchbase_globals, couchbase_globals)
    STD_PHP_INI_ENTRY("couchbase.compression_factor", "1.3",    PHP_INI_ALL, OnUpdateReal, compression_factor, zend_couchbase_globals, couchbase_globals)
    STD_PHP_INI_ENTRY("couchbase.compression_threshold", "2000",    PHP_INI_ALL, OnUpdateLong, compression_threshold, zend_couchbase_globals, couchbase_globals)
PHP_INI_END()
/* }}} */

static char * php_couchbase_zval_to_payload(zval *value, size_t *payload_len, unsigned int *flags, int serializer, int compressor TSRMLS_DC) /* {{{ */ {
    char *payload;
    smart_str buf = {0};

    switch (Z_TYPE_P(value)) {
        case IS_STRING:
            smart_str_appendl(&buf, Z_STRVAL_P(value), Z_STRLEN_P(value));
            *flags = IS_STRING;
            COUCHBASE_SET_COMPRESSION(*flags, compressor);
            break;
        case IS_LONG:
        case IS_DOUBLE:
        case IS_BOOL:
            {
                zval value_copy;
                value_copy = *value;
                zval_copy_ctor(&value_copy);
                convert_to_string(&value_copy);
                smart_str_appendl(&buf, Z_STRVAL(value_copy), Z_STRLEN(value_copy));
                zval_dtor(&value_copy);
                *flags = Z_TYPE_P(value);
                break;
            }
        default:
            COUCHBASE_SET_COMPRESSION(*flags, compressor);
            switch (serializer) {
                case COUCHBASE_SERIALIZER_JSON:
                case COUCHBASE_SERIALIZER_JSON_ARRAY:
#ifdef HAVE_JSON_API
                    {
# if HAVE_JSON_API_5_2
                        php_json_encode(&buf, value TSRMLS_CC);
# elif HAVE_JSON_API_5_3
                        php_json_encode(&buf, value, 0 TSRMLS_CC); /* options */
#endif
                        buf.c[buf.len] = 0;
                        *flags |= COUCHBASE_IS_JSON;
                        break;
                    }
#else
                    php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not serialize value, no json support");
                    return NULL;
#endif
                default:
                    {
                        php_serialize_data_t var_hash;
                        PHP_VAR_SERIALIZE_INIT(var_hash);
                        php_var_serialize(&buf, &value, &var_hash TSRMLS_CC);
                        PHP_VAR_SERIALIZE_DESTROY(var_hash);

                        if (!buf.c) {
                            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not serialize value");
                            smart_str_free(&buf);
                            return NULL;
                        }

                        *flags |= COUCHBASE_IS_SERIALIZED;
                        break;
                    }
            }
            break;
    }

#ifdef HAVE_COMPRESSION
    if ((COUCHBASE_GET_COMPRESSION(*flags)) && buf.len < COUCHBASE_G(compression_threshold)) {
        COUCHBASE_SET_COMPRESSION(*flags, COUCHBASE_COMPRESSION_NONE);
    }

    if (COUCHBASE_GET_COMPRESSION(*flags)) {
        /* status */
        zend_bool compress_status = 0;

        /* Additional 5% for the data */
        size_t payload_comp_len = (size_t)((buf.len * 1.05) + 1);
        char *payload_comp = emalloc(payload_comp_len + sizeof(size_t));
        payload = payload_comp;
        memcpy(payload_comp, &buf.len, sizeof(size_t));
        payload_comp += sizeof(size_t);

        switch (compressor) {
            case COUCHBASE_COMPRESSION_FASTLZ:
#ifdef HAVE_COMPRESSION_FASTLZ
                compress_status = ((payload_comp_len = fastlz_compress(buf.c, buf.len, payload_comp)) > 0);
#else
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not compress value, no fastlz lib support");
                return NULL;
#endif
                break;
             case COUCHBASE_COMPRESSION_ZLIB:
#ifdef HAVE_COMPRESSION_ZLIB
                compress_status = (compress((Bytef *)payload_comp, &payload_comp_len, (Bytef *)buf.c, buf.len) == Z_OK);
#else
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not compress value, no zlib lib support");
                return NULL;
#endif
                break;
             default:
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown compressor type: %d", compressor);
                return NULL;
        }

        if (!compress_status) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not compress value");
            efree(payload);
            smart_str_free(&buf);
            return NULL;
        }

        /* Check that we are above ratio */
        if (buf.len > (payload_comp_len * COUCHBASE_G(compression_factor))) {
            *payload_len = payload_comp_len + sizeof(size_t);
            payload[*payload_len] = 0;
        } else {
            COUCHBASE_SET_COMPRESSION(*flags, COUCHBASE_COMPRESSION_NONE);
            *payload_len = buf.len;
            memcpy(payload, buf.c, buf.len);
            payload[buf.len] = 0;
        }

    } else {
        *payload_len = buf.len;
        payload = estrndup(buf.c, buf.len);
    }
#else
    COUCHBASE_SET_COMPRESSION(*flags, COUCHBASE_COMPRESSION_NONE);
    *payload_len = buf.len;
    payload = estrndup(buf.c, buf.len);
#endif

    smart_str_free(&buf);
    return payload;
}
/* }}} */

static int php_couchbase_zval_from_payload(zval *value, char *payload, size_t payload_len, unsigned int flags, int serializer TSRMLS_DC) /* {{{ */ {
    int compressor;
    zend_bool payload_emalloc = 0;
#ifdef HAVE_COMPRESSION
    char *buffer = NULL;
#endif

    if (payload == NULL && payload_len > 0) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING,
            "could not handle non-existing value of length %zu", payload_len);
        return 0;
    } else if (payload == NULL) {
        if ((flags & 127) == IS_BOOL) {
            ZVAL_FALSE(value);
        } else {
            ZVAL_EMPTY_STRING(value);
        }
        return 1;
    }

    if ((compressor = COUCHBASE_GET_COMPRESSION(flags))) {
#ifdef HAVE_COMPRESSION
        size_t len, length;
        zend_bool decompress_status = 0;
        /* This is copied from pecl-memcached */
        memcpy(&len, payload, sizeof(size_t));
        buffer = emalloc(len + 1);
        payload_len -= sizeof(size_t);
        payload += sizeof(size_t);
        length = len;

        switch (compressor) {
            case COUCHBASE_COMPRESSION_FASTLZ:
#ifdef HAVE_COMPRESSION_FASTLZ
                decompress_status = ((length = fastlz_decompress(payload, payload_len, buffer, len)) > 0);
#else
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value, no fastlz lib support");
                return 0;
#endif
                break;
            case COUCHBASE_COMPRESSION_ZLIB:
#ifdef HAVE_COMPRESSION_ZLIB
                decompress_status = (uncompress((Bytef *)buffer, &length, (Bytef *)payload, payload_len) == Z_OK);
                /* Fall back to 'old style decompression' */
                if (!decompress_status) {
                    unsigned int factor = 1, maxfactor = 16;
                    int status;

                    do {
                        length = (unsigned long)payload_len * (1 << factor++);
                        buffer = erealloc(buffer, length + 1);
                        memset(buffer, 0, length + 1);
                        status = uncompress((Bytef *)buffer, (uLongf *)&length, (const Bytef *)payload, payload_len);
                    } while ((status == Z_BUF_ERROR) && (factor < maxfactor));

                    if (status == Z_OK) {
                        decompress_status = 1;
                    }
                }

#else
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value, no zlib lib support");
                return 0;
#endif
                break;
        }

        if (!decompress_status) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value");
            efree(buffer);
            return 0;
        }

        payload = buffer;
        payload_len = length;
        payload_emalloc = 1;
#else
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not decompress value, no decompressor found");
        return 0;
#endif
    }

    switch (COUCHBASE_GET_TYPE(flags)) {
        case IS_STRING:
            ZVAL_STRINGL(value, payload, payload_len, 1);
            break;

        case 0: /* see http://www.couchbase.com/issues/browse/PCBC-30 */
        case IS_LONG:
        {
            long lval = strtol(payload, NULL, 10);
            ZVAL_LONG(value, lval);
            break;
        }

        case IS_DOUBLE:
        {
            double dval = zend_strtod(payload, NULL);
            ZVAL_DOUBLE(value, dval);
            break;
        }

        case IS_BOOL:
            ZVAL_BOOL(value, payload_len > 0 && payload[0] == '1');
            break;

        case COUCHBASE_IS_SERIALIZED:
        {
            const char *payload_tmp = payload;
            php_unserialize_data_t var_hash;

            PHP_VAR_UNSERIALIZE_INIT(var_hash);
            if (!php_var_unserialize(&value, (const unsigned char **)&payload_tmp, (const unsigned char *)payload_tmp + payload_len, &var_hash TSRMLS_CC)) {
                ZVAL_FALSE(value);
                PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not unserialize value");
                if (payload_emalloc) {
                    efree(payload);
                }
                return 0;
            }
            PHP_VAR_UNSERIALIZE_DESTROY(var_hash);
            break;
        }

        case COUCHBASE_IS_JSON:
#ifdef HAVE_JSON_API
# if HAVE_JSON_API_5_2
            php_json_decode(value, payload, payload_len, (serializer == COUCHBASE_SERIALIZER_JSON_ARRAY) TSRMLS_CC);
# elif HAVE_JSON_API_5_3
            php_json_decode(value, payload, payload_len, (serializer == COUCHBASE_SERIALIZER_JSON_ARRAY), 512 TSRMLS_CC);
# endif
#else
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "could not unserialize value, no json support");
            if (payload_emalloc) {
                efree(payload);
            }
            return 0;
#endif
            break;

        default:
            if (payload_emalloc) {
                efree(payload);
            }
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown payload type");
            return 0;
    }

    if (payload_emalloc) {
        efree(payload);
    }

    return 1;
}
/* }}} */

static void php_couchbase_res_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */ {
    php_couchbase_res *couchbase_res = (php_couchbase_res*)rsrc->ptr;
    if (couchbase_res) {
        if (couchbase_res->handle) {
            libcouchbase_destroy(couchbase_res->handle);
        }
        if (couchbase_res->prefix_key) {
            efree((void *)couchbase_res->prefix_key);
        }
        efree(couchbase_res);
    }
}
/* }}} */

static void php_couchbase_pres_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */ {
    php_couchbase_res *couchbase_res = (php_couchbase_res*)rsrc->ptr;
    if (couchbase_res) {
        if (couchbase_res->handle) {
            libcouchbase_destroy(couchbase_res->handle);
        }
        if (couchbase_res->prefix_key) {
            free((void *)couchbase_res->prefix_key);
        }
        free(couchbase_res);
    }
}
/* }}} */

/* callbacks */
static void php_couchbase_error_callback(libcouchbase_t handle, libcouchbase_error_t error, const char *errinfo) /* {{{ */ {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)libcouchbase_get_cookie(handle);
    /**
     * @FIXME: when connect to a non-couchbase-server port (but the socket is valid)
     * like a apache server, process will be hanged by event_loop
     */
    if (ctx && ctx->res->seqno < 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }
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
                            uint32_t flags, uint64_t cas) {
    zval *retval;
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    TSRMLS_FETCH();
    php_ignore_value(handle);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    ctx->res->rc = error;

    if (LIBCOUCHBASE_SUCCESS != error && LIBCOUCHBASE_KEY_ENOENT != error) {
        ctx->res->io->stop_event_loop(ctx->res->io);
        return;
    }

    if (ctx->res->async) { /* get_delayed */
        zval *k, *v;
        MAKE_STD_ZVAL(v);
        if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
            ctx->res->rc = LIBCOUCHBASE_ERROR;
            efree(v);
            return;
        }

        if (ctx->res->prefix_key_len && nkey) {
            if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
                nkey -= (ctx->res->prefix_key_len + 1); /* '_' */
                key = estrndup(key + ctx->res->prefix_key_len + 1, nkey);
            }
        }

        MAKE_STD_ZVAL(retval);
        array_init(retval);
        zend_hash_next_index_insert(Z_ARRVAL_P(ctx->rv), (void **)&retval, sizeof(zval *), NULL);

        MAKE_STD_ZVAL(k);
        ZVAL_STRINGL(k, (char *)key, nkey, 1);

        zend_hash_add(Z_ARRVAL_P(retval), "key", sizeof("key"), (void **)&k, sizeof(zval *), NULL);
        zend_hash_add(Z_ARRVAL_P(retval), "value", sizeof("value"), (void **)&v, sizeof(zval *), NULL);

        if (ctx->flags) {
            zval *c;
            MAKE_STD_ZVAL(c);
            Z_TYPE_P(c) = IS_STRING;
            Z_STRLEN_P(c) = spprintf(&(Z_STRVAL_P(c)), 0, "%llu", cas);
            zend_hash_add(Z_ARRVAL_P(retval), "cas", sizeof("cas"), (void **)&c, sizeof(zval *), NULL);
        }

        if (ctx->res->prefix_key_len && nkey) {
            efree((void *)key);
        }
    } else {
        if (LIBCOUCHBASE_KEY_ENOENT == error) {
            return;
        }

        if (IS_ARRAY == Z_TYPE_P(ctx->rv)) { /* multi get */
            zval *v;
            MAKE_STD_ZVAL(v);
            if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
                ctx->res->rc = LIBCOUCHBASE_ERROR;
                efree(v);
                return;
           }

            if (ctx->res->prefix_key_len && nkey) {
                if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
                    nkey -= (ctx->res->prefix_key_len + 1);
                    key = estrndup(key + ctx->res->prefix_key_len + 1, nkey);
                }
            }
           zend_hash_add((Z_ARRVAL_P(ctx->rv)), (char *)key, nkey + 1, (void **)&v, sizeof(zval *), NULL);
            if (ctx->cas) {
                zval *c;
                MAKE_STD_ZVAL(c);
                Z_TYPE_P(c) = IS_STRING;
                Z_STRLEN_P(c) = spprintf(&(Z_STRVAL_P(c)), 0, "%llu", cas);
                zend_hash_add(Z_ARRVAL_P(ctx->cas), (char *)key, nkey + 1, (void **)&c, sizeof(zval *), NULL);
            }
            if (ctx->res->prefix_key_len && nkey) {
                efree((void*)key);
            }
        } else {
            if (ctx->res->prefix_key_len && nkey) {
                if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
                    nkey -= (ctx->res->prefix_key_len + 1);
                    key = estrndup(key + ctx->res->prefix_key_len + 1, nkey);
                }
            }
            if (!php_couchbase_zval_from_payload(ctx->rv, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
                if (ctx->res->prefix_key_len && nkey) {
                    efree((void *)key);
                }
                ctx->res->rc = LIBCOUCHBASE_ERROR;
                return;
            }
            if (ctx->res->prefix_key_len && nkey) {
                efree((void *)key);
            }
            if (ctx->cas) {
                Z_TYPE_P(ctx->cas) = IS_STRING;
                Z_STRLEN_P(ctx->cas) = spprintf(&(Z_STRVAL_P(ctx->cas)), 0, "%llu", cas);
            }
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

    ctx->res->rc = error;
    if (error != LIBCOUCHBASE_SUCCESS && error != LIBCOUCHBASE_AUTH_CONTINUE) {
        if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
            zval *rv;
            MAKE_STD_ZVAL(rv);
            ZVAL_FALSE(rv);
            zend_hash_update(Z_ARRVAL_P(ctx->rv), (char *)key, nkey + 1, (void **)&rv, sizeof(zval *), NULL);
        }
        return;
    }

    if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
        zval *rv;
        MAKE_STD_ZVAL(rv);
        Z_TYPE_P(rv) = IS_STRING;
        Z_STRLEN_P(rv) = spprintf(&(Z_STRVAL_P(rv)), 0, "%llu", cas);
        zend_hash_update(Z_ARRVAL_P(ctx->rv), (char *)key, nkey + 1, (void **)&rv, sizeof(zval *), NULL);
    } else {
        Z_TYPE_P(ctx->rv) = IS_STRING;
        Z_STRLEN_P(ctx->rv) = spprintf(&(Z_STRVAL_P(ctx->rv)), 0, "%llu", cas);
    }
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

    ctx->res->rc = error;
}
/* }}} */

/* {{{ static void php_couchbase_flush_callback(...) */
static void
php_couchbase_flush_callback(libcouchbase_t handle,
                            const void* cookie,
                            const char* server_endpoint,
                            libcouchbase_error_t error) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);

    if (--ctx->res->seqno == 0) {
        ctx->res->io->stop_event_loop(ctx->res->io);
    }

    if (server_endpoint) {
        ctx->extended_value = (void *)estrdup(server_endpoint);
    }
    ctx->res->rc = error;
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

    ctx->res->rc = error;
    if (LIBCOUCHBASE_SUCCESS != error) {
        return;
    }

    ZVAL_LONG(ctx->rv, value);
}
/* }}} */

/* {{{ static void php_couchbase_stat_callback(...) */
static void
php_couchbase_stat_callback(libcouchbase_t handle,
                            const void* cookie,
                            const char* server_endpoint,
                            libcouchbase_error_t error,
                            const void* key, size_t nkey,
                            const void* bytes, size_t nbytes) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    php_ignore_value(handle);

    ctx->res->rc = error;
    if (LIBCOUCHBASE_SUCCESS != error || nkey == 0) {
        --ctx->res->seqno;
        ctx->res->io->stop_event_loop(ctx->res->io);
        return;
    } else if (nkey > 0) {
        zval *node, *v;
        zval **ppzval;
        if (IS_ARRAY != Z_TYPE_P(ctx->rv)) {
            array_init(ctx->rv);
        }

        if (zend_hash_find(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&ppzval) == SUCCESS) {
            node = *ppzval;
        } else {
            MAKE_STD_ZVAL(node);
            array_init(node);
            zend_hash_add(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&node, sizeof(zval *), NULL);
        }

        MAKE_STD_ZVAL(v);
        ZVAL_STRINGL(v, (char *)bytes, nbytes, 1);
        zend_hash_add(Z_ARRVAL_P(node), (char *)key, nkey + 1, (void **)&v, sizeof(zval *), NULL);
    }

}
/* }}} */

/* internal implementions */
static void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    char *host;
    char *user = NULL, *passwd = NULL, *bucket = NULL;
    int host_len, user_len, passwd_len, bucket_len;
    zend_bool persistent = 0;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|sssb",
                &host, &host_len, &user, &user_len, &passwd, &passwd_len, &bucket, &bucket_len, &persistent) == FAILURE) {
        return;
    } else {
        libcouchbase_t handle;
        libcouchbase_error_t retval;
        libcouchbase_io_opt_t *iops;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;
        php_url *url = NULL;
        char *hashed_key;
        uint hashed_key_len = 0;

        if (ZEND_NUM_ARGS() == 1 && (strncasecmp(host, "http://", sizeof("http://") - 1) == 0
                || strncasecmp(host, "https://", sizeof("https://") - 1) == 0)) {

             if (!(url = php_url_parse_ex(host, host_len))) {
                 php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
                 RETURN_FALSE;
             }

             if (url->host) {
                 host = url->host;
                 if (url->port) {
                    spprintf(&host, 0, "%s:%d", host, url->port);
                    efree(url->host);
                    url->host = host;
                 }
             } else {
                 php_url_free(url);
                 php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
                 RETURN_FALSE;
             }

             user = url->user;
             passwd = url->pass;
             bucket = url->path;
             if (*bucket == '/') {
                 int i=0, j = strlen(bucket);
                 if (*(bucket + j - 1) == '/') {
                     *(bucket + j - 1) = '\0';
                 }
                 for(;i<j;i++) {
                     bucket[i] = bucket[i+1];
                 }
             }
        }

        if (persistent) {
            zend_rsrc_list_entry *le;
            hashed_key_len = spprintf(&hashed_key, 0, "couchbase_%s_%s_%s_%s", host, user, passwd, bucket);
            if (zend_hash_find(&EG(persistent_list), hashed_key, hashed_key_len + 1, (void **) &le) == FAILURE) {
                goto create_new_link;
            }
            couchbase_res = le->ptr;
            couchbase_res->seqno = 0;
            couchbase_res->async = 0;
            couchbase_res->serializer = COUCHBASE_G(serializer_real);
            couchbase_res->compressor = COUCHBASE_G(compressor_real);
            efree(hashed_key);
        } else {
create_new_link:
            iops = libcouchbase_create_io_ops(LIBCOUCHBASE_IO_OPS_DEFAULT, NULL, NULL);
            if (!iops) {
                if (url) {
                    php_url_free(url);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create IO instance");
                RETURN_FALSE;
            }

            if (!bucket) {
                bucket = "default";
            }

            handle = libcouchbase_create(host, user, passwd, bucket, iops);
            if (!handle) {
                if (url) {
                    php_url_free(url);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create libcouchbase instance");
                RETURN_FALSE;
            }

            php_ignore_value(libcouchbase_set_error_callback(handle, php_couchbase_error_callback));

            if (LIBCOUCHBASE_SUCCESS != (retval = libcouchbase_connect(handle))) {
                if (url) {
                    php_url_free(url);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                        "Failed to connect libcouchbase to server: %s", libcouchbase_strerror(handle, retval));
                RETURN_FALSE;
            }

            php_ignore_value(libcouchbase_set_get_callback(handle, php_couchbase_get_callback));
            php_ignore_value(libcouchbase_set_storage_callback(handle, php_couchbase_storage_callback));
            php_ignore_value(libcouchbase_set_remove_callback(handle, php_couchbase_remove_callback));
            php_ignore_value(libcouchbase_set_flush_callback(handle, php_couchbase_flush_callback));
            php_ignore_value(libcouchbase_set_arithmetic_callback(handle, php_couchbase_arithmetic_callback));
            php_ignore_value(libcouchbase_set_stat_callback(handle, php_couchbase_stat_callback));

            couchbase_res = pecalloc(1, sizeof(php_couchbase_res), persistent);
            couchbase_res->handle = handle;
            couchbase_res->seqno = -1; /* tell error callback stop event loop when error occurred */
            couchbase_res->io = iops;
            couchbase_res->async = 0;
            couchbase_res->serializer = COUCHBASE_G(serializer_real);
            couchbase_res->compressor = COUCHBASE_G(compressor_real);

            ctx = ecalloc(1, sizeof(php_couchbase_ctx));
            ctx->res = couchbase_res;
            libcouchbase_set_cookie(handle, (const void *)ctx);

            /* wait for the connection established */
            libcouchbase_wait(handle);

            couchbase_res->seqno = 0;
            if (LIBCOUCHBASE_SUCCESS != (retval = libcouchbase_get_last_error(handle))) {
                if (url) {
                    php_url_free(url);
                }
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                        "Failed to connect libcouchbase to server: %s", libcouchbase_strerror(handle, retval));
                libcouchbase_destroy(handle);
                efree(couchbase_res);
                efree(ctx);
                RETURN_FALSE;
            }

            if (persistent) {
                zend_rsrc_list_entry le;
                Z_TYPE(le) = le_pcouchbase;
                le.ptr = couchbase_res;
                if (zend_hash_update(&EG(persistent_list), hashed_key, hashed_key_len + 1, (void *) &le, sizeof(zend_rsrc_list_entry), NULL) == FAILURE) {
                    if (url) {
                        php_url_free(url);
                    }
                    php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to store persistent link");
                }
                efree(hashed_key);
                efree(ctx);
            }
        }

        ZEND_REGISTER_RESOURCE(return_value, couchbase_res, persistent? le_pcouchbase : le_couchbase);
        if (oo) {
            zval *self = getThis();
            zend_update_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), return_value TSRMLS_CC);
        }

        if (url) {
            php_url_free(url);
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_get_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo) /* {{{ */ {
    char *key, **keys;
    long *klens, klen = 0;
    int  nkey, flag = 0;
    zval *res, *cas_token = NULL;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
    zend_fcall_info fci = {0};
    zend_fcall_info_cache fci_cache;
#else
    zval *callback = NULL;
#endif
    libcouchbase_error_t retval;
    php_couchbase_res *couchbase_res;
    php_couchbase_ctx *ctx;

    if (multi) {
        zval *akeys;
        zval **ppzval;
        int i;
        if (oo) {
            if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|zl", &akeys, &cas_token, &flag) == FAILURE) {
                return;
            }
            res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
            if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
                RETURN_FALSE;
            }
        } else {
            if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|zl", &res, &akeys, &cas_token, &flag) == FAILURE) {
                return;
            }
        }

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        nkey = zend_hash_num_elements(Z_ARRVAL_P(akeys));
        keys = ecalloc(nkey, sizeof(char *));
        klens = ecalloc(nkey, sizeof(long));

        array_init(return_value);

        for(i=0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
                zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
                zend_hash_move_forward(Z_ARRVAL_P(akeys)), i++) {
            if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void**)&ppzval) == FAILURE) {
                nkey--;
                continue;
            }

            if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
                convert_to_string_ex(ppzval);
            }

            if (!Z_STRLEN_PP(ppzval)) {
                nkey--;
                continue;
            }

            if (couchbase_res->prefix_key_len) {
                klens[i] = spprintf(&(keys[i]), 0, "%s_%s", couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
            } else {
                keys[i] = Z_STRVAL_PP(ppzval);
                klens[i] = Z_STRLEN_PP(ppzval);
            }
        }

        if (!nkey) {
            efree(keys);
            efree(klens);
            return;
        }

        if (cas_token && IS_ARRAY != Z_TYPE_P(cas_token)) {
            zval_dtor(cas_token);
            array_init(cas_token);
        }
    } else {
        if (oo) {
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|f!z", &key, &klen, &fci, &fci_cache, &cas_token) == FAILURE)
#else
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zz", &key, &klen, &callback, &cas_token) == FAILURE)
#endif
            {
               return;
            }
            res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
            if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
                RETURN_FALSE;
            }
        } else {
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|f!z", &res, &key, &klen, &fci, &fci_cache, &cas_token) == FAILURE)
#else
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|zz", &res, &key, &klen, &callback, &cas_token) == FAILURE)
#endif
            {
               return;
            }
        }
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3
        if (callback && Z_TYPE_P(callback) != IS_NULL && !zend_is_callable(callback, 0, NULL)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "third argument is expected to be a valid callback");
            return;
        }
#endif
        if (!klen) {
            return;
        }

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        nkey = 1;
        if (couchbase_res->prefix_key_len) {
            klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
        }
        keys = &key;
        klens = &klen;

        if (cas_token) {
            zval_dtor(cas_token);
            ZVAL_NULL(cas_token);
        }
    }
    {
        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv  = return_value;
        ctx->cas = cas_token;

        retval = libcouchbase_mget(couchbase_res->handle, (const void *)ctx, nkey, (const void *)keys, (size_t *)klens, NULL);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            if (couchbase_res->prefix_key_len) {
                int i;
                for (i=0; i<nkey; i++) {
                    efree(keys[i]);
                }
            }
            if (multi) {
                efree(keys);
                efree(klens);
                zval_dtor(return_value);
            }
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule get request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += nkey;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->res->rc) {
            if (LIBCOUCHBASE_KEY_ENOENT == ctx->res->rc) {
                if (
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
                        fci.size
#else
                        callback
#endif
                   ) {
                    zval *result, *zkey, *retval_ptr = NULL;
                    zval **params[3];

                    MAKE_STD_ZVAL(result);
                    MAKE_STD_ZVAL(zkey);
                    ZVAL_NULL(result);
                    ZVAL_STRINGL(zkey, key, klen, 1);
                    if (oo) {
                        params[0] = &(getThis());
                    } else {
                        params[0] = &res;
                    }
                    params[1] = &zkey;
                    params[2] = &result;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
                    fci.retval_ptr_ptr = &retval_ptr;
                    fci.param_count = 3;
                    fci.params = params;
                    if (zend_call_function(&fci, &fci_cache TSRMLS_CC) == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
                        if (Z_TYPE_P(retval_ptr) == IS_BOOL && Z_BVAL_P(retval_ptr)) {
                            zval_ptr_dtor(fci.retval_ptr_ptr);
                            zval_ptr_dtor(&zkey);
                            efree(ctx);
                            if (multi) {
                                zval_dtor(return_value);
                            }
                            RETURN_ZVAL(result, 1, 1);
                        }
                        zval_ptr_dtor(fci.retval_ptr_ptr);
                    }
#else
                    if (call_user_function_ex(EG(function_table), NULL, callback, &retval_ptr, 3, params, 0, NULL TSRMLS_CC) == SUCCESS) {
                        if (Z_TYPE_P(retval_ptr) == IS_BOOL && Z_BVAL_P(retval_ptr)) {
                            zval_ptr_dtor(&retval_ptr);
                            zval_ptr_dtor(&zkey);
                            efree(ctx);
                            if (multi) {
                                zval_dtor(return_value);
                            }
                            RETURN_ZVAL(result, 1, 1);
                        }
                        zval_ptr_dtor(&retval_ptr);
                    }
#endif
                    zval_ptr_dtor(&zkey);
                    zval_ptr_dtor(&result);
                }
            } else {
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                        "Faild to get a value from server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
            }
        }
        efree(ctx);
        if (couchbase_res->prefix_key_len) {
            int i;
            for (i=0; i<nkey; i++) {
                efree(keys[i]);
            }
        }
        if (multi) {
            efree(keys);
            efree(klens);
        }
    }
}
/* }}} */

static void php_couchbase_get_delayed_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res, *akeys;
    long with_cas = 0;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
    zend_fcall_info fci = {0};
    zend_fcall_info_cache fci_cache;
    if (oo) {
        zval *self = getThis();
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lf!", &akeys, &with_cas, &fci, &fci_cache) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|lf!", &res, &akeys, &with_cas, &fci, &fci_cache) == FAILURE) {
            return;
        }
    }
#else
    zval *callback = NULL;
    if (oo) {
        zval *self = getThis();
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lz", &akeys, &with_cas, &callback) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|lz", &res, &akeys, &with_cas, &callback) == FAILURE) {
            return;
        }
    }
    if (callback && Z_TYPE_P(callback) != IS_NULL
            && !zend_is_callable(callback, 0, NULL)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "third argument is expected to be a valid callback");
        return;
    }
#endif
    {
        zval **ppzval;
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;
        char **keys;
        long nkey, *klens, i;

        nkey = zend_hash_num_elements(Z_ARRVAL_P(akeys));
        keys = ecalloc(nkey, sizeof(char *));
        klens = ecalloc(nkey, sizeof(long));

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

        for(i=0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
                zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
                zend_hash_move_forward(Z_ARRVAL_P(akeys)), i++) {
            if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void**)&ppzval) == FAILURE) {
                nkey--;
                continue;
            }

            if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
                convert_to_string_ex(ppzval);
            }

            if (!Z_STRLEN_PP(ppzval)) {
                nkey--;
                continue;
            }

            if (couchbase_res->prefix_key_len) {
                klens[i] = spprintf(&(keys[i]), 0, "%s_%s", couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
            } else {
                keys[i] = Z_STRVAL_PP(ppzval);
                klens[i] = Z_STRLEN_PP(ppzval);
            }
        }

        if (!nkey) {
            efree(keys);
            efree(klens);
            return;
        }

        couchbase_res->seqno += nkey;
        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->flags = with_cas;

        retval = libcouchbase_mget(couchbase_res->handle, (const void *)ctx, nkey, (const void * const *)keys, (size_t *)klens, NULL);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            if (couchbase_res->prefix_key_len) {
                int i;
                for (i=0; i<nkey; i++) {
                    efree(keys[i]);
                }
            }
            efree(keys);
            efree(klens);
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule delayed get request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }
        couchbase_res->async = 1;
        libcouchbase_set_cookie(couchbase_res->handle, (const void *)ctx);
        if (couchbase_res->prefix_key_len) {
            int i;
            for (i=0; i<nkey; i++) {
                efree(keys[i]);
            }
        }
        efree(keys);
        efree(klens);
        if (
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
                fci.size
#else
                callback
#endif
           ) {
            zval *result, **ppzval, *retval_ptr = NULL;
            zval **params[2];

            MAKE_STD_ZVAL(result);
            array_init(result);
            ctx->rv = result;
            couchbase_res->io->run_event_loop(couchbase_res->io);
            couchbase_res->async = 0;
            for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(result));
                    zend_hash_has_more_elements(Z_ARRVAL_P(result)) == SUCCESS;
                    zend_hash_move_forward(Z_ARRVAL_P(result))) {
                if (zend_hash_get_current_data(Z_ARRVAL_P(result), (void**)&ppzval) == FAILURE) {
                    continue;
                }
                if (oo) {
                    params[0] = &(getThis());
                } else {
                    params[0] = &res;
                }
                params[1] = ppzval;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
                fci.retval_ptr_ptr = &retval_ptr;
                fci.param_count = 2;
                fci.params = params;
                zend_call_function(&fci, &fci_cache TSRMLS_CC);
#else
                call_user_function_ex(EG(function_table), NULL, callback, &retval_ptr, 2, params, 0, NULL TSRMLS_CC);
#endif
                zval_ptr_dtor(&retval_ptr);
            }
            zval_ptr_dtor(&result);
            efree(ctx);
        }
    }
    RETURN_TRUE;
}
/* }}} */

static void php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo) /* {{{ */ {
    zval *res;

    if (oo) {
        zval *self = getThis();
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
            return;
        }
    }
    {
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (!couchbase_res->async) {
            RETURN_FALSE;
        }
        ctx = (php_couchbase_ctx *)libcouchbase_get_cookie(couchbase_res->handle);
        if (couchbase_res->async == 2) {
fetch_one:
            {
                char *key;
                uint key_len;
                ulong index = 0;
                zval **ppzval;
                zval *stash = (zval *)ctx->extended_value;
                if (zend_hash_num_elements(Z_ARRVAL_P(stash)) == 0) {
                    couchbase_res->async = 0;
                    zval_ptr_dtor(&stash);
                    efree(ctx);
                    RETURN_NULL();
                }
                zend_hash_internal_pointer_reset(Z_ARRVAL_P(stash));
                zend_hash_get_current_data(Z_ARRVAL_P(stash), (void **)&ppzval);
                RETVAL_ZVAL(*ppzval, 1, 0);
                zend_hash_get_current_key_ex(Z_ARRVAL_P(stash), &key, &key_len, &index, 0, NULL);
                zend_hash_index_del(Z_ARRVAL_P(stash), index);
                return;
            }
        }

        array_init(return_value);
        ctx->rv = return_value;

        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (!multi) {
            zval *stash;
            MAKE_STD_ZVAL(stash);
            ZVAL_ZVAL(stash, return_value, 1, 0);
            ctx->extended_value = (void *)stash;
            zval_dtor(return_value);
            couchbase_res->async = 2;
            goto fetch_one;
        } else {
            efree(ctx);
            couchbase_res->async = 0;
        }
    }
}
/* }}} */

static void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, libcouchbase_storage_t op, int multi, int oo) /* {{{ */ {
    zval *res, *self;
    libcouchbase_error_t retval;
    php_couchbase_res *couchbase_res;
    php_couchbase_ctx *ctx;
    time_t exp = {0};
    unsigned int flags = 0;
    char *payload, *cas = NULL;
    size_t payload_len = 0;
    unsigned long long cas_v = 0;
    long expire = 0, cas_len = 0;

    self = getThis();
    if (!multi) {
        char *key;
        zval *value;
        long klen = 0;

        if (oo) {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz|ls", &key, &klen, &value, &expire, &cas, &cas_len) == FAILURE) {
                return;
            }
            res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
            if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
                RETURN_FALSE;
            }
        } else {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsz|ls", &res, &key, &klen, &value, &expire, &cas, &cas_len) == FAILURE) {
                return;
            }
        }

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        if (!klen) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to schedule set request: Empty key");
            RETURN_FALSE;
        }

        if (!(payload = php_couchbase_zval_to_payload(value, &payload_len, &flags, couchbase_res->serializer, couchbase_res->compressor TSRMLS_CC))) {
            RETURN_FALSE;
        }

        if (couchbase_res->prefix_key_len) {
            klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv  = return_value;
        couchbase_res->seqno += 1;

        if (expire) {
            exp = expire;
        }

        if (cas) {
            cas_v = strtoull(cas, 0, 10);
        }

        retval = libcouchbase_store(couchbase_res->handle,
                (const void *)ctx, op, key, klen, payload, payload_len, flags, exp, (uint64_t)cas_v);
        
        efree(payload);
        if (couchbase_res->prefix_key_len) {
            efree(key);
        }
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule set request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

    } else {
        zval *akeys, **ppzval;
        char *key;
        uint klen = 0;
        ulong idx;
        int key_type, nkey = 0;

        if (oo) {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|l", &akeys, &expire) == FAILURE) {
                return;
            }
            res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
            if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
                php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
                RETURN_FALSE;
            }
        } else {
            if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|l", &res, &akeys, &expire) == FAILURE) {
                return;
            }
        }

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv  = return_value;
        array_init(ctx->rv);

        if (expire) {
            exp = expire;
        }

        for(zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
                zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
                zend_hash_move_forward(Z_ARRVAL_P(akeys))) {
            if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void**)&ppzval) == FAILURE) {
                continue;
            }
            switch((key_type = zend_hash_get_current_key(Z_ARRVAL_P(akeys), &key, &idx, 0))) {
                case HASH_KEY_IS_LONG:
                    spprintf(&key, 0, "%ld", idx);
                    break;
                case HASH_KEY_IS_STRING:
                    break;
                default:
                    continue;
            }

            if(!(klen = strlen(key))) {
                continue;
            }

            if (!(payload = php_couchbase_zval_to_payload(*ppzval, &payload_len, &flags, couchbase_res->serializer, couchbase_res->compressor TSRMLS_CC))) {
                RETURN_FALSE;
            }

            if (couchbase_res->prefix_key_len) {
                char *new_key;
                klen = spprintf(&new_key, 0, "%s_%s", couchbase_res->prefix_key, key);
                if (key_type == HASH_KEY_IS_LONG) {
                    efree(key);
                }
                key = new_key;
            }

            retval = libcouchbase_store(couchbase_res->handle,
                    (const void *)ctx, op, key, klen, payload, payload_len, flags, exp, 0);

            efree(payload);
            if (couchbase_res->prefix_key_len || HASH_KEY_IS_LONG == key_type) {
                efree(key);
            }
            if (LIBCOUCHBASE_SUCCESS != retval) {
                efree(ctx);
                php_error_docref(NULL TSRMLS_CC, E_WARNING,
                        "Failed to schedule set request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
                RETURN_FALSE;
            }
            nkey++;
        }
        if (!nkey) {
            efree(ctx);
            return;
        }
        couchbase_res->seqno += nkey;
    }
    {
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (IS_ARRAY != Z_TYPE_P(return_value)) {
            if (LIBCOUCHBASE_SUCCESS != ctx->res->rc) {
                RETVAL_FALSE;
                switch (op) {
                    case LIBCOUCHBASE_ADD:
                        if (LIBCOUCHBASE_KEY_EEXISTS == ctx->res->rc) {
                            break;
                        }
                    case LIBCOUCHBASE_APPEND:
                    case LIBCOUCHBASE_PREPEND:
                        if (LIBCOUCHBASE_NOT_STORED == ctx->res->rc) {
                            break;
                        }
                    case LIBCOUCHBASE_REPLACE:
                    case LIBCOUCHBASE_SET:
                        if (LIBCOUCHBASE_KEY_ENOENT == ctx->res->rc) {
                            break;
                        }
                        if (cas && LIBCOUCHBASE_KEY_EEXISTS == ctx->res->rc) {
                            break;
                        }
                    default:
                        php_error_docref(NULL TSRMLS_CC, E_WARNING,
                                "Faild to store a value to server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
                        break;
                }
            }
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res;
    char *key, *cas = NULL;
    long klen = 0, cas_len = 0;
    unsigned long long cas_v = 0;

    if (oo) {
        zval *self = getThis();
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &key, &klen, &cas, &cas_len) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|s", &res, &key, &klen, &cas, &cas_len) == FAILURE) {
            return;
        }
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;

        if (cas) {
            cas_v = strtoull(cas, 0, 10);
        }

        retval = libcouchbase_remove(couchbase_res->handle, (const void *)ctx, (const void * const *)key, klen, cas_v);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule delete request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->res->rc && LIBCOUCHBASE_KEY_ENOENT != ctx->res->rc) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to remove a value from server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
            RETVAL_FALSE;
        } else if (LIBCOUCHBASE_KEY_ENOENT == ctx->res->rc) {
            RETVAL_FALSE;
        } else {
            if (oo) {
                RETVAL_ZVAL(getThis(), 1, 0);
            } else {
                RETVAL_TRUE;
            }
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res;

    if (oo) {
        zval *self = getThis();
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
            return;
        }
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;

        retval = libcouchbase_flush(couchbase_res->handle, (const void *)ctx);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            if(ctx->extended_value) {
                efree(ctx->extended_value);
            }
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule flush request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->res->rc) {
            if(ctx->extended_value) {
                efree(ctx->extended_value);
            }
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to flush node %s: %s", ctx->extended_value?(char *)ctx->extended_value : "", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
            RETURN_FALSE;
        }
        if(ctx->extended_value) {
            efree(ctx->extended_value);
        }
        efree(ctx);
    }

    if (oo) {
        RETURN_ZVAL(getThis(), 1, 0);
    }

    RETURN_TRUE;
}
/* }}} */

static void php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAMETERS, char op, int oo) /* {{{ */ {
    zval *res;
    char *key;
    time_t exp = {0};
    long klen = 0, offset = 1, expire = 0;
    long create = 0, initial = 0;

    if (oo) {
        zval *self = getThis();
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|llll", &key, &klen, &offset, &create, &expire, &initial) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|llll", &res, &key, &klen, &offset, &create, &expire, &initial) == FAILURE) {
            return;
        }
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;
        long delta = (op == '+')? offset : -offset;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        if (expire) {
            exp = expire;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv = return_value;

        retval = libcouchbase_arithmetic(couchbase_res->handle, (const void *)ctx, (const void * const *)key, klen, delta, exp, create, initial);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule rithmetic request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->res->rc) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to %s value in server: %s", (op == '+')? "increment" : "decrement", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
            efree(ctx);
            RETURN_FALSE;
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res;

    if (oo) {
        zval *self = getThis();
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
            return;
        }
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv = return_value;

        retval = libcouchbase_server_stats(couchbase_res->handle, (const void *)ctx, NULL, 0);
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule stat request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LIBCOUCHBASE_SUCCESS != ctx->res->rc) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to stat: %s", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
            RETURN_FALSE;
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res, *value;
    time_t exp = {0};
    unsigned int flags = 0;
    size_t payload_len = 0;
    unsigned long long cas_v = 0;
    char *key, *payload, *cas = NULL;
    long klen = 0, expire = 0, cas_len = 0;

    if (oo) {
        zval *self = getThis();
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ssz|l", &cas, &cas_len, &key, &klen, &value, &expire) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rssz|l", &res, &cas, &cas_len, &key, &klen, &value, &expire) == FAILURE) {
            return;
        }
    }
    {
        libcouchbase_error_t retval;
        php_couchbase_res *couchbase_res;
        php_couchbase_ctx *ctx;

        ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
        if (couchbase_res->async) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
            RETURN_FALSE;
        }

        ctx = ecalloc(1, sizeof(php_couchbase_ctx));
        ctx->res = couchbase_res;
        ctx->rv = return_value;

        if (expire) {
            exp = expire;
        }

        if (cas) {
            cas_v = strtoull(cas, 0, 10);
        }

        if (!(payload = php_couchbase_zval_to_payload(value, &payload_len, &flags, couchbase_res->serializer, couchbase_res->compressor TSRMLS_CC))) {
            RETURN_FALSE;
        }


        if (couchbase_res->prefix_key_len) {
            klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
        }

        retval = libcouchbase_store(couchbase_res->handle, (const void *)ctx,
                LIBCOUCHBASE_SET, key, klen, payload, payload_len, flags, exp, (uint64_t)cas_v);
            
        efree(payload);
        if (couchbase_res->prefix_key_len) {
           efree(key);
        }
        if (LIBCOUCHBASE_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule cas request: %s", libcouchbase_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        ++couchbase_res->seqno;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        zval_dtor(return_value);
        if (LIBCOUCHBASE_SUCCESS == ctx->res->rc) {
            ZVAL_TRUE(return_value);
        } else if (LIBCOUCHBASE_KEY_EEXISTS == ctx->res->rc) {
            ZVAL_FALSE(return_value);
        } else {
            ZVAL_FALSE(return_value);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Faild to store a value to server: %s", libcouchbase_strerror(couchbase_res->handle, ctx->res->rc));
        }
        efree(ctx);
    }
}
/* }}} */

static void php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    long option;
    zval *res, *value;
    php_couchbase_res *couchbase_res;

    if (oo) {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &option, &value) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlz", &res, &option, &value) == FAILURE) {
            return;
        }
    }
    ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

    switch (option) {
        case COUCHBASE_OPT_SERIALIZER:
            {
                convert_to_long_ex(&value);
                switch (Z_LVAL_P(value)) {
                    case COUCHBASE_SERIALIZER_PHP:
                    case COUCHBASE_SERIALIZER_JSON:
                    case COUCHBASE_SERIALIZER_JSON_ARRAY:
#ifdef HAVE_JSON_API
                        couchbase_res->serializer = Z_LVAL_P(value);
                        if (oo) {
                            RETURN_ZVAL(getThis(), 1, 0);
                        }
                        RETURN_TRUE;
#else
                        php_error_docref(NULL TSRMLS_CC, E_WARNING, "json serializer is not supported");
                        RETURN_FALSE;
#endif
                        break;
                    default:
                        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unsupported serializer: %ld", Z_LVAL_P(value));
                }
            }
        case COUCHBASE_OPT_PREFIX_KEY:
            {
                convert_to_string_ex(&value);
                if (couchbase_res->prefix_key) {
                    efree(couchbase_res->prefix_key);
                }
                couchbase_res->prefix_key = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
                couchbase_res->prefix_key_len = Z_STRLEN_P(value);
            }
            break;
        case COUCHBASE_OPT_COMPRESSION:
            {
                convert_to_long_ex(&value);
                switch (Z_LVAL_P(value)) {
                    case COUCHBASE_COMPRESSION_NONE:
                    case COUCHBASE_COMPRESSION_FASTLZ:
                    case COUCHBASE_COMPRESSION_ZLIB:
                        couchbase_res->compressor = Z_LVAL_P(value);
                        if (oo) {
                            RETURN_ZVAL(getThis(), 1, 0);
                        }
                        RETURN_TRUE;
                        break;
                    default:
                        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unsupported compressor: %ld", Z_LVAL_P(value));
                        break;
                }
            }
            break;
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
            break;
    }
    RETURN_FALSE;
}
/* }}} */

static void php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res;
    php_couchbase_res *couchbase_res;
    long option;

    if (oo) {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &option) == FAILURE) {
            return;
        }
        res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
            RETURN_FALSE;
        }
    } else {
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &res, &option) == FAILURE) {
            return;
        }
    }

    ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

    switch (option) {
        case COUCHBASE_OPT_SERIALIZER:
            RETURN_LONG(couchbase_res->serializer);
            break;
        case COUCHBASE_OPT_PREFIX_KEY:
            if (couchbase_res->prefix_key_len) {
                RETURN_STRINGL((char *)couchbase_res->prefix_key, couchbase_res->prefix_key_len, 1);
            } else {
                ZVAL_EMPTY_STRING(return_value);
                return;
            }
            break;
        case COUCHBASE_OPT_COMPRESSION:
            RETURN_LONG(couchbase_res->compressor);
            break;
        default:
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
            break;
    }
    RETURN_FALSE;
}
/* }}} */

/* OO style APIs */
/* {{{ proto Couchbase::__construct(string $host[, string $user[, string $password[, string $bucket[, bool $persistent = false]]]])
*/
PHP_METHOD(couchbase, __construct) {
    php_couchbase_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::get(string $key[, callback $cache_cb[, string &$cas_tokey]])
 */
PHP_METHOD(couchbase, get) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::getMulti(array $keys[, array &cas[, int $flag]])
 */
PHP_METHOD(couchbase, getMulti) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 1);
}
/* }}} */

/* {{{ proto Couchbase::cas(string $cas, string $key, mixed $value[, int $expiration])
 */
PHP_METHOD(couchbase, cas) {
    php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::add(string $key, mixed $value[, int $expiration])
 */
PHP_METHOD(couchbase, add) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_ADD, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::set(string $key, mixed $value[, int $expiration])
 */
PHP_METHOD(couchbase, set) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::setMulti(array $values[, int $expiration])
 */
PHP_METHOD(couchbase, setMulti) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET, 1, 1);
}
/* }}} */

/* {{{ proto Couchbase::prepend(string $key[, string $cas = '0'])
 */
PHP_METHOD(couchbase, prepend) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_PREPEND, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::append(string $key[, string $cas = '0'])
 */
PHP_METHOD(couchbase, append) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_APPEND, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::replace(string $key, mixed $value[, int $expiration[, string $cas = '0']])
 */
PHP_METHOD(couchbase, replace) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_REPLACE, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::increment(string $key[, int $offset = 1[, bool $create = false[, int $expiration = 0[, int $initial = 0]]]])
 */
PHP_METHOD(couchbase, increment) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '+', 1);
}
/* }}} */

/* {{{ proto Couchbase::decrement(string $key[, int $offset = 1[, bool $create = false[, int $expiration = 0[, int $initial = 0]]]])
 */
PHP_METHOD(couchbase, decrement) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '-', 1);
}
/* }}} */

/* {{{ proto Couchbase::getStats(void)
 */
PHP_METHOD(couchbase, getStats) {
    php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::getDelayed(array $keys[, bool $with_cas[, callback $value_cb]])
 */
PHP_METHOD(couchbase, getDelayed) {
    php_couchbase_get_delayed_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::fetch(void)
 */
PHP_METHOD(couchbase, fetch) {
    php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::fetchAll(void)
 */
PHP_METHOD(couchbase, fetchAll) {
    php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 1);
}
/* }}} */

/* {{{ proto Couchbase::delete(string $key[, string $cas = '0'])
 */
PHP_METHOD(couchbase, delete) {
    php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::flush(void)
 */
PHP_METHOD(couchbase, flush) {
    php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::getResultCode(void)
 */
PHP_METHOD(couchbase, getResultCode) {
    zval *res;
    php_couchbase_res *couchbase_res;

    res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
    if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
        RETURN_FALSE;
    }

    ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
    RETURN_LONG(couchbase_res->rc);
}
/* }}} */

/* {{{ proto Couchbase::setOption(int $option, int $value)
 */
PHP_METHOD(couchbase, setOption) {
    php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::getOption(int $option)
 */
PHP_METHOD(couchbase, getOption) {
    php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::getVersion(void)
 */
PHP_METHOD(couchbase, getVersion) {
    RETURN_STRING(PHP_COUCHBASE_VERSION, 1);
}
/* }}} */

/* procedural APIs*/
/* {{{ proto couchbase_connect(string $host[, string $user[, string $password[, string $bucket[, bool $persistent = false]]]])
*/
PHP_FUNCTION(couchbase_connect) {
    php_couchbase_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get(resource $couchbase, string $key[, callback $cache_cb[, string &$cas_tokey]])
 */
PHP_FUNCTION(couchbase_get) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_get_multi(resource $couchbase, array $keys[, array &cas[, int $flag]])
 */
PHP_FUNCTION(couchbase_get_multi) {
    php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
}
/* }}} */

/* {{{ proto couchbase_cas(resource $couchbase, string $cas, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_cas) {
    php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_add(resource $couchbase, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_add) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_ADD, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_set(resource $couchbase, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_set) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_set_multi(resource $couchbase, array $values[, int $expiration])
 */
PHP_FUNCTION(couchbase_set_multi) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_SET, 1, 0);
}
/* }}} */

/* {{{ proto couchbase_prepend(resource $couchbase, string $key[, string $cas = '0'])
 */
PHP_FUNCTION(couchbase_prepend) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_PREPEND, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_append(resource $couchbase, string $key[, string $cas = '0'])
 */
PHP_FUNCTION(couchbase_append) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_APPEND, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_replace(resource $couchbase, string $key, mixed $value[, int $expiration[, string $cas = '0']])
 */
PHP_FUNCTION(couchbase_replace) {
    php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LIBCOUCHBASE_REPLACE, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_increment(resource $couchbase, string $key[, int $offset = 1[, bool $create = false[, int $expiration = 0[, int $initial = 0]]]])
 */
PHP_FUNCTION(couchbase_increment) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '+', 0);
}
/* }}} */

/* {{{ proto couchbase_decrement(resource $couchbase, string $key[, int $offset = 1[, bool $create = false[, int $expiration = 0[, int $initial = 0]]]])
 */
PHP_FUNCTION(couchbase_decrement) {
    php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, '-', 0);
}
/* }}} */

/* {{{ proto couchbase_get_stats(resource $couchbase)
 */
PHP_FUNCTION(couchbase_get_stats) {
    php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_delayed(resource $couchbase, array $keys[, bool $with_cas[, callback $value_cb]])
 */
PHP_FUNCTION(couchbase_get_delayed) {
    php_couchbase_get_delayed_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_fetch(resource $couchbase)
 */
PHP_FUNCTION(couchbase_fetch) {
    php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_fetch_all(resource $couchbase)
 */
PHP_FUNCTION(couchbase_fetch_all) {
    php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
}
/* }}} */

/* {{{ proto couchbase_delete(resource $couchbase, string $key[, string $cas = '0'])
 */
PHP_FUNCTION(couchbase_delete) {
    php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_flush(resource $couchbase)
 */
PHP_FUNCTION(couchbase_flush) {
    php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_result_code(resource $couchbase)
 */
PHP_FUNCTION(couchbase_get_result_code) {
    zval *res;
    php_couchbase_res *couchbase_res;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
        return;
    }
    ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
    RETURN_LONG(couchbase_res->rc);
}
/* }}} */

/* {{{ proto couchbase_set_option(resource $couchbase, int $option, int $value)
 */
PHP_FUNCTION(couchbase_set_option) {
    php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_option(resource $couchbase, int $option)
 */
PHP_FUNCTION(couchbase_get_option) {
    php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_version(void)
 */
PHP_FUNCTION(couchbase_get_version) {
    RETURN_STRING(PHP_COUCHBASE_VERSION, 1);
}
/* }}} */

/* module functions */
/* {{{ PHP_GINIT_FUNCTION
*/
PHP_GINIT_FUNCTION(couchbase) {
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(couchbase) {
    zend_class_entry ce;

    REGISTER_INI_ENTRIES();

    REGISTER_LONG_CONSTANT("COUCHBASE_SUCCESS",         LIBCOUCHBASE_SUCCESS, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_CONTINUE",   LIBCOUCHBASE_AUTH_CONTINUE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_ERROR",      LIBCOUCHBASE_AUTH_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_DELTA_BADVAL",    LIBCOUCHBASE_DELTA_BADVAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_E2BIG",           LIBCOUCHBASE_E2BIG, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EBUSY",           LIBCOUCHBASE_EBUSY, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINTERNAL",       LIBCOUCHBASE_EINTERNAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_EINVAL",          LIBCOUCHBASE_EINVAL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ENOMEM",          LIBCOUCHBASE_ENOMEM, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERANGE",          LIBCOUCHBASE_ERANGE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ERROR",           LIBCOUCHBASE_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_ETMPFAIL",        LIBCOUCHBASE_ETMPFAIL, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_EEXISTS",     LIBCOUCHBASE_KEY_EEXISTS, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_KEY_ENOENT",      LIBCOUCHBASE_KEY_ENOENT, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NETWORK_ERROR",   LIBCOUCHBASE_NETWORK_ERROR, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_MY_VBUCKET",  LIBCOUCHBASE_NOT_MY_VBUCKET, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_STORED",      LIBCOUCHBASE_NOT_STORED, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_NOT_SUPPORTED",   LIBCOUCHBASE_NOT_SUPPORTED, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_COMMAND", LIBCOUCHBASE_UNKNOWN_COMMAND, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_HOST",    LIBCOUCHBASE_UNKNOWN_HOST, CONST_PERSISTENT | CONST_CS);


    REGISTER_LONG_CONSTANT("COUCHBASE_OPT_SERIALIZER",     COUCHBASE_OPT_SERIALIZER, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_OPT_COMPRESSION",     COUCHBASE_OPT_COMPRESSION, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_OPT_PREFIX_KEY",     COUCHBASE_OPT_PREFIX_KEY, CONST_PERSISTENT | CONST_CS);

    REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_PHP",     COUCHBASE_SERIALIZER_PHP, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_JSON",    COUCHBASE_SERIALIZER_JSON, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_JSON_ARRAY", COUCHBASE_SERIALIZER_JSON_ARRAY, CONST_PERSISTENT | CONST_CS);

    REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_NONE", COUCHBASE_COMPRESSION_NONE, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_FASTLZ", COUCHBASE_COMPRESSION_FASTLZ, CONST_PERSISTENT | CONST_CS);
    REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_ZLIB", COUCHBASE_COMPRESSION_ZLIB, CONST_PERSISTENT | CONST_CS);

    le_couchbase = zend_register_list_destructors_ex(php_couchbase_res_dtor, NULL, PHP_COUCHBASE_RESOURCE, module_number);
    le_pcouchbase = zend_register_list_destructors_ex(NULL, php_couchbase_pres_dtor, PHP_COUCHBASE_PERSISTENT_RESOURCE, module_number);

    INIT_CLASS_ENTRY(ce, "Couchbase", couchbase_methods);
    couchbase_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SUCCESS"), LIBCOUCHBASE_SUCCESS TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("AUTH_CONTINUE"), LIBCOUCHBASE_AUTH_CONTINUE TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("AUTH_ERROR"), LIBCOUCHBASE_AUTH_ERROR TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("DELTA_BADVAL"), LIBCOUCHBASE_DELTA_BADVAL TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("E2BIG"), LIBCOUCHBASE_E2BIG TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EBUSY"), LIBCOUCHBASE_EBUSY TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EINTERNAL"), LIBCOUCHBASE_EINTERNAL TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EINVAL"), LIBCOUCHBASE_EINVAL TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ENOMEM"), LIBCOUCHBASE_ENOMEM TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ERANGE"), LIBCOUCHBASE_ERANGE TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ERROR"), LIBCOUCHBASE_ERROR TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ETMPFAIL"), LIBCOUCHBASE_ETMPFAIL TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("KEY_EEXISTS"), LIBCOUCHBASE_KEY_EEXISTS TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("KEY_ENOENT"), LIBCOUCHBASE_KEY_ENOENT TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NETWORK_ERROR"), LIBCOUCHBASE_NETWORK_ERROR TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_MY_VBUCKET"), LIBCOUCHBASE_NOT_MY_VBUCKET TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_STORED"), LIBCOUCHBASE_NOT_STORED TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_SUPPORTED"), LIBCOUCHBASE_NOT_SUPPORTED TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("UNKNOWN_COMMAND"), LIBCOUCHBASE_UNKNOWN_COMMAND TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("UNKNOWN_HOST"), LIBCOUCHBASE_UNKNOWN_HOST TSRMLS_CC);

    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_SERIALIZER"), COUCHBASE_OPT_SERIALIZER TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_COMPRESSION"), COUCHBASE_OPT_COMPRESSION TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_PREFIX_KEY"), COUCHBASE_OPT_PREFIX_KEY TSRMLS_CC);

    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_NONE"), COUCHBASE_COMPRESSION_NONE TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_FASTLZ"), COUCHBASE_COMPRESSION_FASTLZ TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_ZLIB"), COUCHBASE_COMPRESSION_ZLIB TSRMLS_CC);

    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_PHP"), COUCHBASE_SERIALIZER_PHP TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_JSON"), COUCHBASE_SERIALIZER_JSON TSRMLS_CC);
    zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_JSON_ARRAY"), COUCHBASE_SERIALIZER_JSON_ARRAY TSRMLS_CC);

    zend_declare_property_null(couchbase_ce, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), ZEND_ACC_PRIVATE TSRMLS_CC);

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
    php_info_print_table_row(2, "version", PHP_COUCHBASE_VERSION);

#ifdef HAVE_JSON_API
    php_info_print_table_row(2, "json support", "yes");
#else
    php_info_print_table_row(2, "json support", "no");
#endif
#ifdef HAVE_COMPRESSION_FASTLZ
    php_info_print_table_row(2, "fastlz support", "yes");
#else
    php_info_print_table_row(2, "fastlz support", "no");
#endif
#ifdef HAVE_COMPRESSION_ZLIB
    php_info_print_table_row(2, "zlib support", "yes");
#else
    php_info_print_table_row(2, "zlib support", "no");
#endif

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
