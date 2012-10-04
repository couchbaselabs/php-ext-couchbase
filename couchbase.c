/*
  +----------------------------------------------------------------------+
  | PHP Version 5														 |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.										 |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");		 |
  | you may not use this file except in compliance with the License.	 |
  | You may obtain a copy of the License at								 |
  |		http://www.apache.org/licenses/LICENSE-2.0						 |
  | Unless required by applicable law or agreed to in writing, software	 |
  | distributed under the License is distributed on an "AS IS" BASIS,	 |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or		 |
  | implied. See the License for the specific language governing		 |
  | permissions and limitations under the License.						 |
  +----------------------------------------------------------------------+
  | Author: Xinchen Hui	   <laruence@php.net>							 |
  +----------------------------------------------------------------------+
*/

/* $Id */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_var.h"
#ifdef HAVE_JSON_API
# include "ext/json/php_json.h"
#endif
#include "ext/standard/php_var.h"
#include <libcouchbase/couchbase.h>
#include "php_couchbase.h"
#ifdef HAVE_COMPRESSION_FASTLZ
# include "fastlz.c"
#endif
#ifdef HAVE_COMPRESSION_ZLIB
# include "zlib.h"
#endif


#include "Zend/zend_API.h"

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
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_multi, 0, 0, 2)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_ARRAY_INFO(1, cas_tokens, 1)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_delayed, 0, 0, 2)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_INFO(0, with_cas)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_touch, 0, 0, 3)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, expiry)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_touch_multi, 0, 0, 3)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_INFO(0, expiry)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_view, 0, 0, 2)
	ZEND_ARG_INFO(0, resource)
	ZEND_ARG_INFO(0, doc_name)
	ZEND_ARG_INFO(0, view_name)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
	ZEND_ARG_INFO(0, return_errors)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_result_message, 0, 0, 1)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_version, 0, 0, 1)
    ZEND_ARG_INFO(0, resource)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_get_client_version, 0, 0, 0)
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
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getmulti, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_ARRAY_INFO(1, cas_tokens, 1)
	ZEND_ARG_INFO(0, flags)
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_getdelayed, 0, 0, 1)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_INFO(0, with_cas)
	ZEND_ARG_INFO(0, cb)
	ZEND_ARG_INFO(0, expiry)
	ZEND_ARG_INFO(0, lock)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_touch, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, expiry)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_touchmulti, 0, 0, 2)
	ZEND_ARG_ARRAY_INFO(0, keys, 0)
	ZEND_ARG_INFO(0, expiry)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_fetch, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_fetchall, 0, 0, 0)
ZEND_END_ARG_INFO()

COUCHBASE_ARG_PREFIX
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_view, 0, 0, 1)
	ZEND_ARG_INFO(0, doc_name)
	ZEND_ARG_INFO(0, view_name)
	ZEND_ARG_ARRAY_INFO(0, options, 0)
	ZEND_ARG_INFO(0, return_errors)
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
ZEND_BEGIN_ARG_INFO_EX(arginfo_m_resultmessage, 0, 0, 0)
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
	PHP_FE(couchbase_touch, arginfo_touch)
	PHP_FE(couchbase_touch_multi, arginfo_touch_multi)
	PHP_FE(couchbase_fetch, arginfo_fetch)
	PHP_FE(couchbase_fetch_all, arginfo_fetch_all)
	PHP_FE(couchbase_view, arginfo_view)
	PHP_FE(couchbase_increment, arginfo_increment)
	PHP_FE(couchbase_decrement, arginfo_decrement)
	PHP_FE(couchbase_get_stats, arginfo_get_stats)
	PHP_FE(couchbase_delete, arginfo_delete)
	PHP_FE(couchbase_flush, arginfo_flush)
	PHP_FE(couchbase_get_result_code, arginfo_result_code)
	PHP_FE(couchbase_get_result_message, arginfo_result_message)
	PHP_FE(couchbase_set_option, arginfo_set_option)
	PHP_FE(couchbase_get_option, arginfo_get_option)
	PHP_FE(couchbase_get_version, arginfo_get_version)
	PHP_FE(couchbase_get_client_version, arginfo_get_client_version)
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
	PHP_ME(couchbase, touch, arginfo_m_touch, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, touchMulti, arginfo_m_touchmulti, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, fetch, arginfo_m_fetch, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, fetchAll, arginfo_m_fetchall, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, view, arginfo_m_view, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, delete, arginfo_m_delete, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getStats, arginfo_m_getstats, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, flush, arginfo_m_flush, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, increment, arginfo_m_increment, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, decrement, arginfo_m_decrement, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getResultCode, arginfo_m_resultcode, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getResultMessage, arginfo_m_resultmessage, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, setOption, arginfo_m_setoption, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getOption, arginfo_m_getoption, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getVersion, arginfo_get_version, ZEND_ACC_PUBLIC)
	PHP_ME(couchbase, getClientVersion, arginfo_get_client_version, ZEND_ACC_PUBLIC)
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
	PHP_RINIT(couchbase),		 /* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(couchbase),	 /* Replace with NULL if there's nothing to do at request end */
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
	STD_PHP_INI_ENTRY("couchbase.compressor", "none",	 PHP_INI_ALL, OnUpdateCompressor, compressor, zend_couchbase_globals, couchbase_globals)
	STD_PHP_INI_ENTRY("couchbase.compression_factor", "1.3",	PHP_INI_ALL, OnUpdateReal, compression_factor, zend_couchbase_globals, couchbase_globals)
	STD_PHP_INI_ENTRY("couchbase.compression_threshold", "2000",	PHP_INI_ALL, OnUpdateLong, compression_threshold, zend_couchbase_globals, couchbase_globals)
PHP_INI_END()
/* }}} */

/**
 * Include helper source files
 */
#include "views.c"

static char * php_couchbase_zval_to_payload(zval *value, size_t *payload_len, unsigned int *flags, int serializer, int compressor TSRMLS_DC) /* {{{ */ {
	char *payload;
	smart_str buf = {0};

	switch (Z_TYPE_P(value)) {
		case IS_STRING:
			smart_str_appendl(&buf, Z_STRVAL_P(value), Z_STRLEN_P(value));
			COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_STRING);
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
				switch (Z_TYPE_P(value)) {
				case IS_LONG:
					COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_LONG);
					break;
				case IS_DOUBLE:
					COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_DOUBLE);
					break;
				case IS_BOOL:
					COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_BOOL);
					break;
				}
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
						COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_JSON);
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

						COUCHBASE_VAL_SET_TYPE(*flags, COUCHBASE_VAL_IS_SERIALIZED);
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
				{
					uLongf tmp_ulen;
					compress_status = (compress((Bytef *)payload_comp, &tmp_ulen, (Bytef *)buf.c, buf.len) == Z_OK);
					/* sync with payload_comp_len */
					payload_comp_len = tmp_ulen;
				}
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
				{
					uLongf ulength = length;
					decompress_status = (uncompress((Bytef *)buffer, &ulength, (Bytef *)payload, payload_len) == Z_OK);
					/* Fall back to 'old style decompression' */
					if (!decompress_status) {
						unsigned int factor = 1, maxfactor = 16;
						int status;

						do {
							length = (unsigned long)payload_len * (1 << factor++);
							buffer = erealloc(buffer, length + 1);
							memset(buffer, 0, length + 1);
							status = uncompress((Bytef *)buffer, &ulength, (const Bytef *)payload, payload_len);
						} while ((status == Z_BUF_ERROR) && (factor < maxfactor));

						if (status == Z_OK) {
							decompress_status = 1;
						}
					}
					length = ulength;
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

	switch (COUCHBASE_VAL_GET_TYPE(flags)) {
		case COUCHBASE_VAL_IS_STRING:
			ZVAL_STRINGL(value, payload, payload_len, 1);
			break;

		//case 0: /* see http://www.couchbase.com/issues/browse/PCBC-30 */
		case COUCHBASE_VAL_IS_LONG:
		{
			char *buf = emalloc(payload_len + sizeof(char));
			memcpy(buf, payload, payload_len);
			buf[payload_len] = '\0';
			long lval = strtol(buf, NULL, 10);
			efree(buf);
			ZVAL_LONG(value, lval);
			break;
		}

		case COUCHBASE_VAL_IS_DOUBLE:
		{
			char *buf = emalloc(payload_len + sizeof(char));
			memcpy(buf, payload, payload_len);
			buf[payload_len] = '\0';
			double dval = zend_strtod(payload, NULL);
			efree(buf);
			ZVAL_DOUBLE(value, dval);
			break;
		}

		case COUCHBASE_VAL_IS_BOOL:
			ZVAL_BOOL(value, payload_len > 0 && payload[0] == '1');
			break;

		case COUCHBASE_VAL_IS_SERIALIZED:
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

		case COUCHBASE_VAL_IS_JSON:
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
			lcb_destroy(couchbase_res->handle);
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
			lcb_destroy(couchbase_res->handle);
		}
		if (couchbase_res->prefix_key) {
			free((void *)couchbase_res->prefix_key);
		}
		free(couchbase_res);
	}
}
/* }}} */

/* callbacks */
static void php_couchbase_error_callback(lcb_t handle, lcb_error_t error, const char *errinfo) /* {{{ */ {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)lcb_get_cookie(handle);
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
php_couchbase_get_callback(lcb_t instance,
						   const void *cookie,
						   lcb_error_t error,
						   const lcb_get_resp_t *resp) {
	zval *retval;
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const void *key;
	size_t nkey;
	const void *bytes;
	size_t nbytes;
	uint32_t flags;
	uint64_t cas;


	TSRMLS_FETCH();
	php_ignore_value(instance);

	if (--ctx->res->seqno == 0) {
		ctx->res->io->stop_event_loop(ctx->res->io);
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}
	key = resp->v.v0.key;
	nkey = resp->v.v0.nkey;
	bytes = resp->v.v0.bytes;
	nbytes = resp->v.v0.nbytes;
	flags = resp->v.v0.flags;
	cas = resp->v.v0.cas;

	ctx->res->rc = error;
	if (LCB_SUCCESS != error && LCB_KEY_ENOENT != error) {
		ctx->res->io->stop_event_loop(ctx->res->io);
		return;
	}

	if (ctx->res->async) { /* get_delayed */
		zval *k, *v;
		MAKE_STD_ZVAL(v);
		if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
			ctx->res->rc = LCB_ERROR;
			efree(v);
			return;
		}

		if (ctx->res->prefix_key_len && nkey) {
			if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
				nkey -= (ctx->res->prefix_key_len + 1); /* '_' */
				key = estrndup(((const char *)key) + ctx->res->prefix_key_len + 1, nkey);
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
		if (LCB_KEY_ENOENT == error) {
			return;
		}

		if (IS_ARRAY == Z_TYPE_P(ctx->rv)) { /* multi get */
			zval *v;
			char *key_string;
			MAKE_STD_ZVAL(v);
			if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
				ctx->res->rc = LCB_ERROR;
				efree(v);
				return;
		   }

			if (ctx->res->prefix_key_len && nkey) {
				if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
					nkey -= (ctx->res->prefix_key_len + 1);
					key_string = estrndup(((const char *)key) + ctx->res->prefix_key_len + 1, nkey);
				}
			} else {
				key_string = emalloc(nkey + 1);
				memcpy(key_string, key, nkey);
				key_string[nkey] = '\0';
			}
		   zend_hash_update((Z_ARRVAL_P(ctx->rv)), (char *)key_string, nkey + 1, (void **)&v, sizeof(zval *), NULL);

			if (ctx->cas) {
				zval *c;
				MAKE_STD_ZVAL(c);
				Z_TYPE_P(c) = IS_STRING;
				Z_STRLEN_P(c) = spprintf(&(Z_STRVAL_P(c)), 0, "%llu", cas);
				zend_hash_add(Z_ARRVAL_P(ctx->cas), (char *)key_string, nkey + 1, (void **)&c, sizeof(zval *), NULL);
			}
			efree(key_string);
		} else {
			if (ctx->res->prefix_key_len && nkey) {
				if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
					nkey -= (ctx->res->prefix_key_len + 1);
					key = estrndup(((const char *)key) + ctx->res->prefix_key_len + 1, nkey);
				}
			}
			if (!php_couchbase_zval_from_payload(ctx->rv, (char *)bytes, nbytes, flags, ctx->res->serializer TSRMLS_CC)) {
				if (ctx->res->prefix_key_len && nkey) {
					efree((void *)key);
				}
				ctx->res->rc = LCB_ERROR;
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

/* {{{ static void php_couchbase_touch_callback(...)
 */
static void
php_couchbase_touch_callback(lcb_t handle,
                           const void *cookie,
                           lcb_error_t error,
                           const lcb_touch_resp_t *resp) {
    php_ignore_value(handle);
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    const char* key = (char *)resp->v.v0.key;
    lcb_size_t nkey = resp->v.v0.nkey;
	char *string_key;

    // TODO: is cas needed? existing php docs don't say anything about it being used, but it's in the resp struct...
    // lcb_cas_t cas = resp->v.v0.cas;

	if (--ctx->res->seqno == 0) {
		ctx->res->io->stop_event_loop(ctx->res->io);
	}

	ctx->res->rc = error;

	if (LCB_SUCCESS != error || key == NULL || nkey == 0) {
        return;
    } else if (nkey > 0) {
        if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
            // set (key name => true) within return value associative array (we did touch it)
		    string_key = emalloc(nkey + 1);
			memcpy(string_key, key, nkey);
			string_key[nkey] = '\0';

            add_assoc_bool(ctx->rv, string_key, (zend_bool)1);

			efree(string_key);
        } else {
            // set return val to true (we touched the one thing we set out to touch)
            ZVAL_BOOL(ctx->rv, 1);
        }
    }
}
/* }}} */

/* {{{ static void php_couchbase_storage_callback(...)
 */
static void
php_couchbase_store_callback(lcb_t instance,
							 const void *cookie,
							 lcb_storage_t operation,
							 lcb_error_t error,
							 const lcb_store_resp_t *resp) {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const void *key;
	size_t nkey;
	uint64_t cas;
	php_ignore_value(instance);
	php_ignore_value(operation);

	if (--ctx->res->seqno == 0) {
		ctx->res->io->stop_event_loop(ctx->res->io);
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}
	key = resp->v.v0.key;
	nkey = resp->v.v0.nkey;
	cas = resp->v.v0.cas;

	ctx->res->rc = error;
	if (error != LCB_SUCCESS && error != LCB_AUTH_CONTINUE) {
		if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
			zval *rv;
			char *string_key = emalloc(nkey + 1);
			memcpy(string_key, key, nkey);
			string_key[nkey] = '\0';

			MAKE_STD_ZVAL(rv);
			ZVAL_FALSE(rv);
			zend_hash_update(Z_ARRVAL_P(ctx->rv), string_key, nkey + 1, (void **)&rv, sizeof(zval *), NULL);
			efree(string_key);
		}
		return;
	}

	if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
		zval *rv;
		char *string_key = emalloc(nkey + 1);
		memcpy(string_key, key, nkey);
		string_key[nkey] = '\0';

		MAKE_STD_ZVAL(rv);
		Z_TYPE_P(rv) = IS_STRING;
		Z_STRLEN_P(rv) = spprintf(&(Z_STRVAL_P(rv)), 0, "%llu", cas);
		zend_hash_update(Z_ARRVAL_P(ctx->rv), string_key, nkey + 1, (void **)&rv, sizeof(zval *), NULL);
		efree(string_key);
	} else {
		Z_TYPE_P(ctx->rv) = IS_STRING;
		Z_STRLEN_P(ctx->rv) = spprintf(&(Z_STRVAL_P(ctx->rv)), 0, "%llu", cas);
	}
}
/* }}} */

/* {{{ static void php_couchbase_remove_callback(...) */
static void
php_couchbase_remove_callback(lcb_t instance,
							  const void *cookie,
							  lcb_error_t error,
							  const lcb_remove_resp_t *resp) {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	php_ignore_value(instance);
	php_ignore_value(resp);

	if (--ctx->res->seqno == 0) {
		ctx->res->io->stop_event_loop(ctx->res->io);
	}

	ctx->res->rc = error;
}
/* }}} */

/* {{{ static void php_couchbase_flush_callback(...) */
static void php_couchbase_flush_callback(lcb_t handle,
                                         const void* cookie,
                                         lcb_error_t error,
                                         const lcb_flush_resp_t *resp) {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    const char* server_endpoint = resp->v.v0.server_endpoint;
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
php_couchbase_arithmetic_callback(lcb_t instance,
								  const void *cookie,
								  lcb_error_t error,
								  const lcb_arithmetic_resp_t *resp) {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	uint64_t value;
	php_ignore_value(instance);

	if (--ctx->res->seqno == 0) {
		ctx->res->io->stop_event_loop(ctx->res->io);
	}

	ctx->res->rc = error;
	if (LCB_SUCCESS != error) {
		return;
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}

	ZVAL_LONG(ctx->rv,	resp->v.v0.value);
}
/* }}} */

/* {{{ static void php_couchbase_stat_callback(...) */
static void php_couchbase_stat_callback(lcb_t handle,
                                        const void* cookie,
                                        lcb_error_t error,
                                        const lcb_server_stat_resp_t *resp) {
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    const char* server_endpoint = resp->v.v0.server_endpoint;
    const void* key = resp->v.v0.key;
    size_t nkey = resp->v.v0.nkey;
    const void* bytes = resp->v.v0.bytes;
    size_t nbytes = resp->v.v0.nbytes;

	php_ignore_value(handle);

	ctx->res->rc = error;
	if (LCB_SUCCESS != error || nkey == 0) {
		--ctx->res->seqno;
		ctx->res->io->stop_event_loop(ctx->res->io);
		return;
	} else if (nkey > 0) {
		zval *node, *val;
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

		char *string_key = emalloc(nkey + 1);
		memcpy(string_key, key, nkey);
		string_key[nkey] = '\0';

		MAKE_STD_ZVAL(val);
		ZVAL_STRINGL(val, (char *)bytes, nbytes, 1);

		zend_hash_add(Z_ARRVAL_P(node), string_key, nkey + 1, (void **)&val, sizeof(zval *), NULL);
		efree(string_key);
	}
}
/* }}} */

/* {{{ static void php_couchbase_version_callback(...) */
static void
php_couchbase_version_callback(lcb_t handle,
                               const void* cookie,
                               lcb_error_t error,
                               const lcb_server_version_resp_t *resp) {
    php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
    const char* server_endpoint = resp->v.v0.server_endpoint;
	const char* version_string = resp->v.v0.vstring;
	lcb_size_t nversion_string = resp->v.v0.nvstring;
    php_ignore_value(handle);

    ctx->res->rc = error;
    if (LCB_SUCCESS != error || nversion_string == 0 || server_endpoint == NULL) {
        --ctx->res->seqno;
        ctx->res->io->stop_event_loop(ctx->res->io);
        return;
    } else if (nversion_string > 0) {
        zval *v;
        zval **ppzval;

        if (IS_ARRAY != Z_TYPE_P(ctx->rv)) {
            array_init(ctx->rv);
        }

        if (zend_hash_find(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&ppzval) != SUCCESS) {
            MAKE_STD_ZVAL(v);
            ZVAL_STRINGL(v, version_string, nversion_string, 1);
            zend_hash_add(Z_ARRVAL_P(ctx->rv), (char *)server_endpoint, strlen(server_endpoint) + 1, (void **)&v, sizeof(zval *), NULL);
        }
    }
}
/* }}} */

struct php_couchbase_nodeinfo_st;

struct php_couchbase_nodeinfo_st {
    struct php_couchbase_nodeinfo_st *next;
    char *host;
    php_url *url;
};

struct php_couchbase_connparams_st {
    struct php_couchbase_nodeinfo_st *nodes;
    struct php_couchbase_nodeinfo_st *tail;

    char *host_string;
    char *bucket;
    char *username;
    char *password;
};

static int php_couchbase_parse_host(const char *host,
                                       size_t host_len,
                                       struct php_couchbase_connparams_st *cparams TSRMLS_DC)
{
    php_url *url = NULL;
    struct php_couchbase_nodeinfo_st *curnode;
    curnode = ecalloc(1, sizeof(*curnode));

    if (!cparams->tail) {
        cparams->nodes = cparams->tail = curnode;
    } else {
        cparams->tail->next = curnode;
        cparams->tail = curnode;
    }


    if (strncasecmp(host, "http://", sizeof("http://") - 1) != 0
                    && strncasecmp(host, "https://", sizeof("https://") - 1)
                    != 0)
    {
        /* simple host string */
        curnode->host = ecalloc(1, host_len + 1);
        memcpy(curnode->host, host, host_len);
        curnode->host[host_len] = '\0';
        return 1;
    }


    if (!(url = php_url_parse_ex(host, host_len))) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
        return 0;
    }

    if (!url->host) {
        php_url_free(url);
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "malformed host url %s", host);
        return 0;
    }

    curnode->url = url;
    return 1;
}

static int _append_host_port(char *oldstr, char **newstr,
                             const char *host,
                             unsigned short port)
{
    if (oldstr) {
        if (port) {
            return spprintf(newstr, 0, "%s;%s:%d", oldstr, host, port);
        } else {
            return spprintf(newstr, 0, "%s;%s", oldstr, host);
        }
    } else {
        if (port) {
            return spprintf(newstr, 0, "%s:%d", host, port);
        } else {
            return spprintf(newstr, 0, "%s", host);
        }
    }
}

static long _check_expiry(long expiry) {
	if (expiry < 0) {
		php_error(E_RECOVERABLE_ERROR, "Expiry must not be negative (%d given).", expiry);
	}
	return expiry;
}

static void php_couchbase_make_params(struct php_couchbase_connparams_st *cparams)
{
    struct php_couchbase_nodeinfo_st *ni;
    char *curstr = NULL;
    int curlen;

    for (ni = cparams->nodes; ni; ni = ni->next) {

        char *newstr = NULL;
        char *curhost;

        if (ni->url) {

            _append_host_port(curstr, &newstr, ni->url->host, ni->url->port);

            if (cparams->username == NULL) {
                cparams->username = ni->url->user;
            }

            if (cparams->password == NULL) {
                cparams->password = ni->url->pass;
            }

            if (cparams->bucket == NULL && ni->url->path != NULL && ni->url->path[0] == '/') {

                char *bucket = ni->url->path;
                int i=0, j = strlen(bucket);

                if (*(bucket + j - 1) == '/') {
                    *(bucket + j - 1) = '\0';
                }

                for(;i<j;i++) {
                    bucket[i] = bucket[i+1];
                }

                cparams->bucket = bucket;
            }

        } else {
            _append_host_port(curstr, &newstr, ni->host, 0);
        }

        if (curstr != NULL) {
            efree(curstr);
        }
        curstr = newstr;
    }
    cparams->host_string = curstr;
}

static void php_couchbase_free_connparams(
        struct php_couchbase_connparams_st *cparams)
{
    struct php_couchbase_nodeinfo_st *ni = cparams->nodes;
    while (ni) {

        struct php_couchbase_nodeinfo_st *next = ni->next;
        if (ni->url) {
            php_url_free(ni->url);
        } else if (ni->host) {
            efree(ni->host);
        }

        efree(ni);
        ni = next;
    }

    if (cparams->host_string) {
        efree(cparams->host_string);
    }
}

/* internal implementions */
static void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
	char *user = NULL, *passwd = NULL, *bucket = NULL;
	int host_len = 0, user_len = 0, passwd_len = 0, bucket_len = 0;
	zend_bool persistent = 0;
	zval *zvhosts = NULL;
	struct php_couchbase_connparams_st cparams = { NULL };

	memset(&cparams, 0, sizeof(cparams));

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|zsssb",
				&zvhosts,
				&user, &user_len,
				&passwd, &passwd_len,
				&bucket, &bucket_len,
				&persistent) == FAILURE) {
		return;
	} else {
		lcb_t handle;
		lcb_error_t retval;
		lcb_io_opt_t iops;
		php_couchbase_res *couchbase_res;
		php_couchbase_ctx *ctx = NULL;
		char *hashed_key;
		uint hashed_key_len = 0;

		if (ZEND_NUM_ARGS() >= 4) {
		    if (bucket_len) {
		        cparams.bucket = bucket;
		    }
		    if (user_len) {
		        cparams.username = user;
		    }
		    if (passwd_len) {
		        cparams.password = passwd;
		    }
		}
		if (zvhosts == NULL) {
			char host_localhost[] = "127.0.0.1";
			php_couchbase_parse_host(host_localhost, sizeof(host_localhost)-1,
					&cparams TSRMLS_CC);

		} else if (Z_TYPE_P(zvhosts) == IS_STRING) {
		    if (!php_couchbase_parse_host(
		            Z_STRVAL_P(zvhosts), Z_STRLEN_P(zvhosts), &cparams TSRMLS_CC)) {
		        php_couchbase_free_connparams(&cparams);
		        RETURN_FALSE;
		    }
		} else if (Z_TYPE_P(zvhosts) == IS_ARRAY) {
		    int nhosts;
		    zval **curzv = NULL;
		    HashTable *hthosts = Z_ARRVAL_P(zvhosts);
		    HashPosition htpos;
		    int ii;
		    nhosts = zend_hash_num_elements(hthosts);

		    for (ii = 0, zend_hash_internal_pointer_reset_ex(hthosts, &htpos);
		            ii < nhosts &&
		            zend_hash_get_current_data_ex(hthosts, (void**)&curzv, &htpos) == SUCCESS;
		            zend_hash_move_forward_ex(hthosts, &htpos), ii++) {
		        if (!Z_TYPE_PP(curzv) == IS_STRING) {

		            php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                             "Couldn't get string from node lists");
		            php_couchbase_free_connparams(&cparams);
		            RETURN_FALSE;
		        }
		        if (!php_couchbase_parse_host(Z_STRVAL_PP(curzv),
		                                      Z_STRLEN_PP(curzv),
		                                      &cparams TSRMLS_CC)) {
		            php_couchbase_free_connparams(&cparams);
		            RETURN_FALSE;
		        }
		    }

		} else {
		    php_error_docref(NULL TSRMLS_CC, E_WARNING,
		                     "Hosts is neither a string nor an array");
		    php_couchbase_free_connparams(&cparams);
		    RETURN_FALSE;
		}

        php_couchbase_make_params(&cparams);


		if (persistent) {
			zend_rsrc_list_entry *le;
			hashed_key_len = spprintf(&hashed_key, 0, "couchbase_%s_%s_%s_%s",
			                          cparams.host_string, user, passwd, bucket);
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
            struct lcb_create_st create_options;

create_new_link:
            if (lcb_create_io_ops(&iops, NULL) != LCB_SUCCESS) {
			    php_couchbase_free_connparams(&cparams);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "failed to create IO instance");
				RETURN_FALSE;
			}

			if (!cparams.bucket) {
			    cparams.bucket = "default";
			}

            memset(&create_options.version, 0, sizeof(create_options));
            create_options.v.v0.host = cparams.host_string;
            create_options.v.v0.user = cparams.username;
            create_options.v.v0.passwd = cparams.password;
            create_options.v.v0.bucket = cparams.bucket;
            create_options.v.v0.io = iops;

			if (lcb_create(&handle, &create_options) != LCB_SUCCESS) {
			    php_couchbase_free_connparams(&cparams);
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to create libcouchbase instance");
				RETURN_FALSE;
			}

			php_ignore_value(lcb_set_error_callback(handle, php_couchbase_error_callback));

			if (LCB_SUCCESS != (retval = lcb_connect(handle))) {
			    php_couchbase_free_connparams(&cparams);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Failed to connect libcouchbase to server: %s", lcb_strerror(handle, retval));
				RETURN_FALSE;
			}

			php_ignore_value(lcb_set_get_callback(handle, php_couchbase_get_callback));
			php_ignore_value(lcb_set_store_callback(handle, php_couchbase_store_callback));
			php_ignore_value(lcb_set_remove_callback(handle, php_couchbase_remove_callback));
			php_ignore_value(lcb_set_flush_callback(handle, php_couchbase_flush_callback));
			php_ignore_value(lcb_set_arithmetic_callback(handle, php_couchbase_arithmetic_callback));
			php_ignore_value(lcb_set_stat_callback(handle, php_couchbase_stat_callback));
			php_ignore_value(lcb_set_version_callback(handle, php_couchbase_version_callback));
			php_ignore_value(lcb_set_view_complete_callback(handle, php_couchbase_complete_callback));
			php_ignore_value(lcb_set_touch_callback(handle, php_couchbase_touch_callback));

			couchbase_res = pecalloc(1, sizeof(php_couchbase_res), persistent);
			couchbase_res->handle = handle;
			couchbase_res->seqno = -1; /* tell error callback stop event loop when error occurred */
			couchbase_res->io = iops;
			couchbase_res->async = 0;
			couchbase_res->serializer = COUCHBASE_G(serializer_real);
			couchbase_res->compressor = COUCHBASE_G(compressor_real);

			ctx = ecalloc(1, sizeof(php_couchbase_ctx));
			ctx->res = couchbase_res;
			lcb_set_cookie(handle, (const void *)ctx);

			/* wait for the connection established */
			lcb_wait(handle);

			couchbase_res->seqno = 0;
			if (LCB_SUCCESS != (retval = lcb_get_last_error(handle))) {
			    php_couchbase_free_connparams(&cparams);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Failed to connect libcouchbase to server: %s", lcb_strerror(handle, retval));
				lcb_destroy(handle);
				pefree(couchbase_res, persistent);
				efree(ctx);
				ctx = NULL;
				RETURN_FALSE;
			}

			if (persistent) {
				zend_rsrc_list_entry le;
				Z_TYPE(le) = le_pcouchbase;
				le.ptr = couchbase_res;
				if (zend_hash_update(&EG(persistent_list), hashed_key, hashed_key_len + 1, (void *) &le, sizeof(zend_rsrc_list_entry), NULL) == FAILURE) {
				    php_couchbase_free_connparams(&cparams);
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to store persistent link");
				}
				efree(hashed_key);
				efree(ctx);
				ctx = NULL;
			}
		}

		ZEND_REGISTER_RESOURCE(return_value, couchbase_res, persistent? le_pcouchbase : le_couchbase);
		if (oo) {
			zval *self = getThis();
			zend_update_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), return_value TSRMLS_CC);
		}
		php_couchbase_free_connparams(&cparams);
        if (ctx != NULL) {
            efree(ctx);
        }
    }
}
/* }}} */

static void php_couchbase_get_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo) /* {{{ */ {
	char *key, **keys;
	long *klens, klen = 0;
	int	 nkey, flag = 0;
	lcb_time_t exp = {0};
	long expiry = 0;
	zval *res, *cas_token = NULL;
	zend_bool lock = 0;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
	zend_fcall_info fci = {0};
	zend_fcall_info_cache fci_cache;
#else
	zval *callback = NULL;
#endif
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;

	if (multi) {
		zval *akeys;
		zval **ppzval;
		zend_bool preserve_order;
		int i;
		if (oo) {
			if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|zllb", &akeys, &cas_token, &flag, &expiry, &lock) == FAILURE) {
				return;
			}
			res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
			if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
				RETURN_FALSE;
			}
		} else {
			if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|zllb", &res, &akeys, &cas_token, &flag, &expiry, &lock) == FAILURE) {
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
		preserve_order = (flag & COUCHBASE_GET_PRESERVE_ORDER);

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

			if(preserve_order) {
				add_assoc_null_ex(return_value, keys[i], klens[i] + 1);
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
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|f!zlb", &key, &klen, &fci, &fci_cache, &cas_token, &expiry, &lock) == FAILURE)
#else
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|zzlb", &key, &klen, &callback, &cas_token, &expiry, &lock) == FAILURE)
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
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|f!zlb", &res, &key, &klen, &fci, &fci_cache, &cas_token, &expiry, &lock) == FAILURE)
#else
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|zzlb", &res, &key, &klen, &callback, &cas_token, &expiry, &lock) == FAILURE)
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
		lcb_get_cmd_t** commands = ecalloc(nkey, sizeof(lcb_get_cmd_t*));
		int ii;

		if (expiry) {
			exp = _check_expiry(expiry);
		}

		for (ii = 0; ii < nkey; ++ii) {
			lcb_get_cmd_t* cmd = ecalloc(1, sizeof(lcb_get_cmd_t));
			commands[ii] = cmd;
			cmd->v.v0.key = keys[ii];
			cmd->v.v0.nkey = klens[ii];
			cmd->v.v0.lock = (int)lock;
			cmd->v.v0.exptime = exp; /* NB: this assumes sizeof(lcb_time_t) == sizeof(long) */
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv	 = return_value;
		ctx->cas = cas_token;

		retval = lcb_get(couchbase_res->handle, ctx,
						 nkey, (const lcb_get_cmd_t* const *)commands);
		for (ii = 0; ii < nkey; ++ii) {
			efree(commands[ii]);
		}
		efree(commands);

		if (LCB_SUCCESS != retval) {
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
					"Failed to schedule get request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += nkey;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS != ctx->res->rc) {
			if (LCB_KEY_ENOENT == ctx->res->rc) {
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
						"Failed to get a value from server: %s", lcb_strerror(couchbase_res->handle, ctx->res->rc));
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
	lcb_time_t exp = {0};
	long expiry = 0;
	zend_bool lock = 0;
#if PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 2
	zend_fcall_info fci = {0};
	zend_fcall_info_cache fci_cache;

	if (oo) {
		zval *self = getThis();
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lf!lb", &akeys, &with_cas, &fci, &fci_cache, &expiry, &lock) == FAILURE) {
			return;
		}
		res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|lf!lb", &res, &akeys, &with_cas, &fci, &fci_cache, &expiry, &lock) == FAILURE) {
			return;
		}
	}
#else
	zval *callback = NULL;
	if (oo) {
		zval *self = getThis();
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a|lzlb", &akeys, &with_cas, &callback, &expiry, &lock) == FAILURE) {
			return;
		}
		res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ra|lzlb", &res, &akeys, &with_cas, &callback, &expiry, &lock) == FAILURE) {
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
		lcb_error_t retval;
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

		{
			lcb_get_cmd_t** commands = ecalloc(nkey, sizeof(lcb_get_cmd_t*));
			int ii;

			if (expiry) {
				exp = _check_expiry(expiry);
			}

			for (ii = 0; ii < nkey; ++ii) {
				lcb_get_cmd_t* cmd = ecalloc(1, sizeof(lcb_get_cmd_t));
				commands[ii] = cmd;
				cmd->v.v0.key = keys[ii];
				cmd->v.v0.nkey = klens[ii];
				cmd->v.v0.lock = (int)lock;
				cmd->v.v0.exptime = exp; /* NB: this assumes that sizeof(lcb_time_t) == sizeof(long) */
			}

			retval = lcb_get(couchbase_res->handle, ctx,
							 nkey, (const lcb_get_cmd_t* const *)commands);
			for (ii = 0; ii < nkey; ++ii) {
				efree(commands[ii]);
			}
			efree(commands);
		}

		if (LCB_SUCCESS != retval) {
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
					"Failed to schedule delayed get request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}
		couchbase_res->async = 1;
		lcb_set_cookie(couchbase_res->handle, (const void *)ctx);
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

static void php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo) /* {{{ */ {
	char *key;   /* for a single key */
	long nkey;   /* (size of key string) */
	char **keys; /* for an array of keys */
	long nkeys;  /* (size of array of key strings, and of array of key strings' sizes) */
	long *keyslens; /* (array of sizes of key strings) */
	lcb_time_t exp = {0}; /* how long to set expiry. */
	long expiry; /* used for parameter passing */
	/* note that by server's behavior, anything longer than 30 days (60*60*24*30) is an epoch time to expire at */
	zval *res;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;

	/* parameter handling and return_value setup: */

	if (multi) {
		zval *arr_keys, *fv;
		zval **ppzval;
		int i;

		if (oo) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "al", &arr_keys, &expiry) == FAILURE) {
				return;
			}
			res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
			if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "uninitialized couchbase");
				RETURN_FALSE;
			}
		} else { /* multi-and-not-oo */
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ral", &res, &arr_keys, &expiry) == FAILURE) {
				return;
			}
		}

		ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
		if (couchbase_res->async) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results that should be fetched before doing any sync request");
			RETURN_FALSE;
		}

		nkeys = zend_hash_num_elements(Z_ARRVAL_P(arr_keys));
		keys = ecalloc(nkeys, sizeof(char *));
		keyslens = ecalloc(nkeys, sizeof(long));

		array_init(return_value);

		for (i = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr_keys));
				zend_hash_has_more_elements(Z_ARRVAL_P(arr_keys)) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(arr_keys)), i++) {
			if (zend_hash_get_current_data(Z_ARRVAL_P(arr_keys), (void**)&ppzval) == FAILURE) {
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
				keyslens[i] = spprintf(&(keys[i]), 0, "%s_%s", couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
			} else {
				keys[i] = Z_STRVAL_PP(ppzval);
				keyslens[i] = Z_STRLEN_PP(ppzval);
			}

			/* set keyname => false in the return array (will get set to true in the touch callback when/if keyname seen) */
            add_assoc_bool(return_value, keys[i], (zend_bool)0);
		}

		if (!nkey) {
			efree(keys);
			efree(keyslens);
			return;
		}
	} else { /* single-valued */
		if (oo) {
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sl", &key, &nkey, &expiry) == FAILURE) {
				return;
			}
			res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
			if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintialized couchbase");
				RETURN_FALSE;
			}
		} else { /* single-valued-and-not-oo */
			if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rsl", &res, &key, &nkey, &expiry) == FAILURE) {
				return;
			}
		}

		if (!nkey) {
			return;
		}

		ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
		if (couchbase_res->async) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results that should be fetched before doing any sync request");
			RETURN_FALSE;
		}

		nkeys = 1;
		if (couchbase_res->prefix_key_len) {
			nkey = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
		}
		keys = &key;
		keyslens = &nkey;

		/* set return value false, will get set to true in the touch callback when/if the operation succeeds */
		ZVAL_FALSE(return_value);
	}

	/* main action */
	{
		lcb_touch_cmd_t** commands = ecalloc(nkeys, sizeof(lcb_touch_cmd_t*));
		lcb_error_t retval;
		int ii;

		if (expiry) {
			exp = _check_expiry(expiry);
		}

		for (ii = 0; ii < nkeys; ++ii) {
			lcb_touch_cmd_t* cmd = ecalloc(1, sizeof(lcb_touch_cmd_t));
			cmd->version = 0;
			cmd->v.v0.key = keys[ii];
			cmd->v.v0.nkey = keyslens[ii];
			cmd->v.v0.exptime = exp; /* note: this assumes sizeof(long) == sizeof(lcb_time_t) */
			commands[ii] = cmd;
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		retval = lcb_touch(couchbase_res->handle, ctx, nkeys, (const lcb_touch_cmd_t* const *)commands);
		for (ii = 0; ii < nkeys; ++ii) {
			efree(commands[ii]);
		}
		efree(commands);

		if (LCB_SUCCESS != retval) {
			if (couchbase_res->prefix_key_len) {
				int i;
				for (i = 0; i < nkeys; i++) {
					efree(keys[i]);
				}
			}

			if (multi) {
				efree(keys);
				efree(keyslens);
				zval_dtor(return_value);
			}

			efree(ctx);

			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to schedule touch request: %s",
					lcb_strerror(couchbase_res->handle, retval));

			RETURN_FALSE;
		}

		couchbase_res->seqno += nkeys;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS != ctx->res->rc) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed touch request: %s",
					lcb_strerror(couchbase_res->handle, ctx->res->rc));
		}

		efree(ctx);
		if (couchbase_res->prefix_key_len) {
			int i;
			for (i = 0; i < nkeys; i++) {
				efree(keys[i]);
			}
		}

		if (multi) {
			efree(keys);
			efree(keyslens);
		}
	}
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
		ctx = (php_couchbase_ctx *)lcb_get_cookie(couchbase_res->handle);
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

static void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, lcb_storage_t op, int multi, int oo) /* {{{ */ {
	zval *res, *self;
	lcb_error_t retval;
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
		ctx->rv	 = return_value;
		couchbase_res->seqno += 1;

		if (expire) {
			exp = _check_expiry(expire);
		}

		if (cas) {
			cas_v = strtoull(cas, 0, 10);
		}

		{
			lcb_store_cmd_t cmd;
			lcb_store_cmd_t* commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.operation = op;
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.bytes = payload;
			cmd.v.v0.nbytes = payload_len;
			cmd.v.v0.flags = flags;
			cmd.v.v0.exptime = exp;
			cmd.v.v0.cas = (uint64_t)cas_v;

			retval = lcb_store(couchbase_res->handle, ctx,
							   1, (const lcb_store_cmd_t * const *)commands);
		}

		efree(payload);
		if (couchbase_res->prefix_key_len) {
			efree(key);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule set request: %s", lcb_strerror(couchbase_res->handle, retval));
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
		ctx->rv	 = return_value;
		array_init(ctx->rv);

		if (expire) {
			exp = _check_expiry(expire);
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

			{
				lcb_store_cmd_t cmd;
				lcb_store_cmd_t* commands[] = { &cmd };
				memset(&cmd, 0, sizeof(cmd));
				cmd.v.v0.operation = op;
				cmd.v.v0.key = key;
				cmd.v.v0.nkey = klen;
				cmd.v.v0.bytes = payload;
				cmd.v.v0.nbytes = payload_len;
				cmd.v.v0.flags = flags;
				cmd.v.v0.exptime = exp;

				retval = lcb_store(couchbase_res->handle, ctx,
								   1, (const lcb_store_cmd_t * const *)commands);
			}

			efree(payload);
			if (couchbase_res->prefix_key_len || HASH_KEY_IS_LONG == key_type) {
				efree(key);
			}
			if (LCB_SUCCESS != retval) {
				efree(ctx);
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Failed to schedule set request: %s", lcb_strerror(couchbase_res->handle, retval));
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
			if (LCB_SUCCESS != ctx->res->rc) {
				RETVAL_FALSE;
				switch (op) {
					case LCB_ADD:
						if (LCB_KEY_EEXISTS == ctx->res->rc) {
							break;
						}
					case LCB_APPEND:
					case LCB_PREPEND:
						if (LCB_NOT_STORED == ctx->res->rc) {
							break;
						}
					case LCB_REPLACE:
					case LCB_SET:
						if (LCB_KEY_ENOENT == ctx->res->rc) {
							break;
						}
						if (cas && LCB_KEY_EEXISTS == ctx->res->rc) {
							break;
						}
					default:
						php_error_docref(NULL TSRMLS_CC, E_WARNING,
								"Failed to store a value to server: %s", lcb_strerror(couchbase_res->handle, ctx->res->rc));
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
		lcb_error_t retval;
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

		{
			lcb_remove_cmd_t cmd;
			lcb_remove_cmd_t* commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.cas = cas_v;
			retval = lcb_remove(couchbase_res->handle, ctx,
								1, (const lcb_remove_cmd_t * const*)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule delete request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS == ctx->res->rc) {
			if (oo) {
				RETVAL_ZVAL(getThis(), 1, 0);
			} else {
				RETVAL_TRUE;
			}
		} else if (LCB_KEY_ENOENT == ctx->res->rc ||	/* skip missing key errors */
				   LCB_KEY_EEXISTS == ctx->res->rc) {	/* skip CAS mismatch */
			RETVAL_FALSE;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to remove a value from server: %s", lcb_strerror(couchbase_res->handle, ctx->res->rc));
			RETVAL_FALSE;
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
		lcb_error_t retval;
		php_couchbase_res *couchbase_res;
		php_couchbase_ctx *ctx;

		ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
		if (couchbase_res->async) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
			RETURN_FALSE;
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
        {
            lcb_flush_cmd_t cmd;
            lcb_flush_cmd_t *commands[] = { &cmd };
            memset(&cmd, 0, sizeof(cmd));
            retval = lcb_flush(couchbase_res->handle, (const void *)ctx,
                               1, (const lcb_flush_cmd_t * const *)commands);
        }
		if (LCB_SUCCESS != retval) {
			if(ctx->extended_value) {
				efree(ctx->extended_value);
			}
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule flush request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS != ctx->res->rc) {
			if(ctx->extended_value) {
				efree(ctx->extended_value);
			}
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to flush node %s: %s", ctx->extended_value?(char *)ctx->extended_value : "", lcb_strerror(couchbase_res->handle, ctx->res->rc));
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
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|llll", &key, &klen, &offset, &create, &expire, &initial) == FAILURE) {
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
		lcb_error_t retval;
		php_couchbase_res *couchbase_res;
		php_couchbase_ctx *ctx;
		long delta = (op == '+')? offset : -offset;

		ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
		if (couchbase_res->async) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
			RETURN_FALSE;
		}

		if (expire) {
			exp = _check_expiry(expire);
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_arithmetic_cmd_t cmd;
			lcb_arithmetic_cmd_t* commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.create = create;
			cmd.v.v0.delta = delta;
			cmd.v.v0.initial = initial;
			cmd.v.v0.exptime = exp;

			retval = lcb_arithmetic(couchbase_res->handle, ctx, 1,
									(const lcb_arithmetic_cmd_t * const*)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule rithmetic request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS != ctx->res->rc) {
			// Just return false and don't print a warning when no key is present and create is false
			if(!(LCB_KEY_ENOENT == ctx->res->rc && create == 0)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
						"Failed to %s value in server: %s", (op == '+')? "increment" : "decrement", lcb_strerror(couchbase_res->handle, ctx->res->rc));
			}
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
		lcb_error_t retval;
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

        {
            lcb_server_stats_cmd_t cmd;
            lcb_server_stats_cmd_t* commands[] = { &cmd };
            memset(&cmd, 0, sizeof(cmd));
            retval = lcb_server_stats(couchbase_res->handle, (const void *)ctx,
                                      1, (const lcb_server_stats_cmd_t * const *)commands);
        }
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule stat request: %s", retval, lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		if (LCB_SUCCESS != ctx->res->rc) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to stat: %s",ctx->res->rc, lcb_strerror(couchbase_res->handle, ctx->res->rc));
			efree(ctx);
			RETURN_FALSE;
		}
		efree(ctx);
	}
}
/* }}} */

static void php_couchbase_version_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
    zval *res;

    if (oo) {
        zval *self = getThis();
        res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
        if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "uninitialized couchbase");
            RETURN_FALSE;
        }
    } else {
        if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
            return;
        }
    }
    {
        lcb_error_t retval;
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

        {
			lcb_server_version_cmd_t cmd;
            lcb_server_version_cmd_t* commands[] = { &cmd };
            memset(&cmd, 0, sizeof(cmd));
            retval = lcb_server_versions(couchbase_res->handle, (const void *)ctx,
                                      1, (const lcb_server_version_cmd_t * const *)commands);
        }
        if (LCB_SUCCESS != retval) {
            efree(ctx);
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to schedule server version request: %s", lcb_strerror(couchbase_res->handle, retval));
            RETURN_FALSE;
        }

        couchbase_res->seqno += 1;
        couchbase_res->io->run_event_loop(couchbase_res->io);
        if (LCB_SUCCESS != ctx->res->rc) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING,
                    "Failed to fetch server version (%u): %s", ctx->res->rc, lcb_strerror(couchbase_res->handle, ctx->res->rc));
            efree(ctx);
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
		lcb_error_t retval;
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
			exp = _check_expiry(expire);
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

		{
			lcb_store_cmd_t cmd;
			lcb_store_cmd_t* commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.operation = LCB_SET;
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.bytes = payload;
			cmd.v.v0.nbytes = payload_len;
			cmd.v.v0.flags = flags;
			cmd.v.v0.exptime = exp;
			cmd.v.v0.cas = (uint64_t)cas_v;

			retval = lcb_store(couchbase_res->handle, ctx,
							   1, (const lcb_store_cmd_t * const *)commands);
		}

		efree(payload);
		if (couchbase_res->prefix_key_len) {
		   efree(key);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to schedule cas request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		++couchbase_res->seqno;
		couchbase_res->io->run_event_loop(couchbase_res->io);
		zval_dtor(return_value);
		if (LCB_SUCCESS == ctx->res->rc) {
			ZVAL_TRUE(return_value);
		} else if (LCB_KEY_EEXISTS == ctx->res->rc) {
			ZVAL_FALSE(return_value);
		} else {
			ZVAL_FALSE(return_value);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
					"Failed to store a value to server: %s", lcb_strerror(couchbase_res->handle, ctx->res->rc));
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

static void php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */ {
	zval *res;
	php_couchbase_res *couchbase_res;
	char *str;
	int str_len;

	if(oo) {
		res = zend_read_property(couchbase_ce, getThis(), ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
			return;
		}
	}

	ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
	str_len = spprintf(&str, 0, "%s", lcb_strerror(couchbase_res->handle, couchbase_res->rc));
	RETURN_STRINGL(str, str_len, 0);
}
/* }}} */


/* OO style APIs */
/* {{{ proto Couchbase::__construct(string $host[, string $user[, string $password[, string $bucket[, bool $persistent = false]]]])
*/
PHP_METHOD(couchbase, __construct) {
	php_couchbase_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::get(string $key[, callback $cache_cb[, string &$cas_tokey[, int $expiry[, bool $lock]]]])
 */
PHP_METHOD(couchbase, get) {
	php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::getMulti(array $keys[, array &cas[, int $flag[, int $expiry[, bool $lock]]]])
 */
PHP_METHOD(couchbase, getMulti) {
	php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 1);
}
/* }}} */

/* {{{ proto Couchbase::touch(string $key, int $expiry)
 */
PHP_METHOD(couchbase, touch) {
	php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::touchMulti(array $keys, int $expiry)
 */
PHP_METHOD(couchbase, touchMulti) {
	php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 1);
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
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_ADD, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::set(string $key, mixed $value[, int $expiration])
 */
PHP_METHOD(couchbase, set) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_SET, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::setMulti(array $values[, int $expiration])
 */
PHP_METHOD(couchbase, setMulti) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_SET, 1, 1);
}
/* }}} */

/* {{{ proto Couchbase::prepend(string $key[, string $cas = '0'])
 */
PHP_METHOD(couchbase, prepend) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_PREPEND, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::append(string $key[, string $cas = '0'])
 */
PHP_METHOD(couchbase, append) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_APPEND, 0, 1);
}
/* }}} */

/* {{{ proto Couchbase::replace(string $key, mixed $value[, int $expiration[, string $cas = '0']])
 */
PHP_METHOD(couchbase, replace) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_REPLACE, 0, 1);
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

/* {{{ proto Couchbase::getDelayed(array $keys[, bool $with_cas[, callback $value_cb[, int $expiry[, bool $locking]]]])
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

/* {{{ proto Couchbase::view(string $doc_name, string $view_name[, array $options])
 */
PHP_METHOD(couchbase, view) {
	php_couchbase_view_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
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

/* {{{ proto Couchbase::getResultMessage(void)
 */
PHP_METHOD(couchbase, getResultMessage) {
	php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
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
    php_couchbase_version_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1);
}
/* }}} */

/* {{{ proto Couchbase::getClientVersion(void)
 */
PHP_METHOD(couchbase, getClientVersion) {
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

/* {{{ proto couchbase_get(resource $couchbase, string $key[, callback $cache_cb[, string &$cas_tokey[, int $expiry[, bool $lock]]]])
 */
PHP_FUNCTION(couchbase_get) {
	php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_get_multi(resource $couchbase, array $keys[, array &cas[, int $flag[, int $expiry[, bool $lock]]]])
 */
PHP_FUNCTION(couchbase_get_multi) {
	php_couchbase_get_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
}
/* }}} */

/* {{{ proto couchbase_touch(resource $couchbase, string $key, int $expiry)
 */
PHP_FUNCTION(couchbase_touch) {
	php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_touch_multi(resource $couchbase, array $keys, int $expiry)
 */
PHP_FUNCTION(couchbase_touch_multi) {
	php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 1, 0);
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
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_ADD, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_set(resource $couchbase, string $key, mixed $value[, int $expiration])
 */
PHP_FUNCTION(couchbase_set) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_SET, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_set_multi(resource $couchbase, array $values[, int $expiration])
 */
PHP_FUNCTION(couchbase_set_multi) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_SET, 1, 0);
}
/* }}} */

/* {{{ proto couchbase_prepend(resource $couchbase, string $key[, string $cas = '0'])
 */
PHP_FUNCTION(couchbase_prepend) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_PREPEND, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_append(resource $couchbase, string $key[, string $cas = '0'])
 */
PHP_FUNCTION(couchbase_append) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_APPEND, 0, 0);
}
/* }}} */

/* {{{ proto couchbase_replace(resource $couchbase, string $key, mixed $value[, int $expiration[, string $cas = '0']])
 */
PHP_FUNCTION(couchbase_replace) {
	php_couchbase_store_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, LCB_REPLACE, 0, 0);
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

/* {{{ proto couchbase_get_delayed(resource $couchbase, array $keys[, bool $with_cas[, callback $value_cb[, int $expiry[, bool $lock]]]])
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

/* {{{ proto couchbase_view(resource $couchbase, string $doc_name, string $view_name[, array $options])
 */
PHP_FUNCTION(couchbase_view) {
	php_couchbase_view_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
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

/* {{{ proto couchbase_get_result_message(resource $couchbase)
 */
PHP_FUNCTION(couchbase_get_result_message) {
	php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
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
	php_couchbase_version_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU, 0);
}
/* }}} */

/* {{{ proto couchbase_get_client_version(void)
 */
PHP_FUNCTION(couchbase_get_client_version) {
	RETURN_STRING(PHP_COUCHBASE_VERSION, 1);
}

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

	REGISTER_LONG_CONSTANT("COUCHBASE_SUCCESS",			LCB_SUCCESS, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_CONTINUE",	LCB_AUTH_CONTINUE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_AUTH_ERROR",		LCB_AUTH_ERROR, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_DELTA_BADVAL",	LCB_DELTA_BADVAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_E2BIG",			LCB_E2BIG, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_EBUSY",			LCB_EBUSY, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_EINTERNAL",		LCB_EINTERNAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_EINVAL",			LCB_EINVAL, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_ENOMEM",			LCB_ENOMEM, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_ERANGE",			LCB_ERANGE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_ERROR",			LCB_ERROR, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_ETMPFAIL",		LCB_ETMPFAIL, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_KEY_EEXISTS",		LCB_KEY_EEXISTS, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_KEY_ENOENT",		LCB_KEY_ENOENT, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_NETWORK_ERROR",	LCB_NETWORK_ERROR, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_NOT_MY_VBUCKET",	LCB_NOT_MY_VBUCKET, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_NOT_STORED",		LCB_NOT_STORED, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_NOT_SUPPORTED",	LCB_NOT_SUPPORTED, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_COMMAND", LCB_UNKNOWN_COMMAND, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_UNKNOWN_HOST",	LCB_UNKNOWN_HOST, CONST_PERSISTENT | CONST_CS);


	REGISTER_LONG_CONSTANT("COUCHBASE_OPT_SERIALIZER",	   COUCHBASE_OPT_SERIALIZER, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_OPT_COMPRESSION",		COUCHBASE_OPT_COMPRESSION, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_OPT_PREFIX_KEY",	   COUCHBASE_OPT_PREFIX_KEY, CONST_PERSISTENT | CONST_CS);

	REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_PHP",	   COUCHBASE_SERIALIZER_PHP, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_JSON",	   COUCHBASE_SERIALIZER_JSON, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_SERIALIZER_JSON_ARRAY", COUCHBASE_SERIALIZER_JSON_ARRAY, CONST_PERSISTENT | CONST_CS);

	REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_NONE", COUCHBASE_COMPRESSION_NONE, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_FASTLZ", COUCHBASE_COMPRESSION_FASTLZ, CONST_PERSISTENT | CONST_CS);
	REGISTER_LONG_CONSTANT("COUCHBASE_COMPRESSION_ZLIB", COUCHBASE_COMPRESSION_ZLIB, CONST_PERSISTENT | CONST_CS);

	REGISTER_LONG_CONSTANT("COUCHBASE_GET_PRESERVE_ORDER", COUCHBASE_GET_PRESERVE_ORDER, CONST_PERSISTENT | CONST_CS);

	le_couchbase = zend_register_list_destructors_ex(php_couchbase_res_dtor, NULL, PHP_COUCHBASE_RESOURCE, module_number);
	le_pcouchbase = zend_register_list_destructors_ex(NULL, php_couchbase_pres_dtor, PHP_COUCHBASE_PERSISTENT_RESOURCE, module_number);

	INIT_CLASS_ENTRY(ce, "Couchbase", couchbase_methods);
	couchbase_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SUCCESS"), LCB_SUCCESS TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("AUTH_CONTINUE"), LCB_AUTH_CONTINUE TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("AUTH_ERROR"), LCB_AUTH_ERROR TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("DELTA_BADVAL"), LCB_DELTA_BADVAL TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("E2BIG"), LCB_E2BIG TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EBUSY"), LCB_EBUSY TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EINTERNAL"), LCB_EINTERNAL TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("EINVAL"), LCB_EINVAL TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ENOMEM"), LCB_ENOMEM TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ERANGE"), LCB_ERANGE TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ERROR"), LCB_ERROR TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("ETMPFAIL"), LCB_ETMPFAIL TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("KEY_EEXISTS"), LCB_KEY_EEXISTS TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("KEY_ENOENT"), LCB_KEY_ENOENT TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NETWORK_ERROR"), LCB_NETWORK_ERROR TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_MY_VBUCKET"), LCB_NOT_MY_VBUCKET TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_STORED"), LCB_NOT_STORED TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("NOT_SUPPORTED"), LCB_NOT_SUPPORTED TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("UNKNOWN_COMMAND"), LCB_UNKNOWN_COMMAND TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("UNKNOWN_HOST"), LCB_UNKNOWN_HOST TSRMLS_CC);

	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_SERIALIZER"), COUCHBASE_OPT_SERIALIZER TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_COMPRESSION"), COUCHBASE_OPT_COMPRESSION TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("OPT_PREFIX_KEY"), COUCHBASE_OPT_PREFIX_KEY TSRMLS_CC);

	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_NONE"), COUCHBASE_COMPRESSION_NONE TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_FASTLZ"), COUCHBASE_COMPRESSION_FASTLZ TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("COMPRESSION_ZLIB"), COUCHBASE_COMPRESSION_ZLIB TSRMLS_CC);

	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_PHP"), COUCHBASE_SERIALIZER_PHP TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_JSON"), COUCHBASE_SERIALIZER_JSON TSRMLS_CC);
	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("SERIALIZER_JSON_ARRAY"), COUCHBASE_SERIALIZER_JSON_ARRAY TSRMLS_CC);

	zend_declare_class_constant_long(couchbase_ce, ZEND_STRL("GET_PRESERVE_ORDER"), COUCHBASE_GET_PRESERVE_ORDER TSRMLS_CC);

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
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
