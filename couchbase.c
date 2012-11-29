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

#include "internal.h"

PHP_COUCHBASE_LOCAL
void pcbc_start_loop(struct _php_couchbase_res *res)
{
	lcb_wait(res->handle);
}

PHP_COUCHBASE_LOCAL
void pcbc_stop_loop(struct _php_couchbase_res *res)
{
	lcb_breakout(res->handle);
}


PHP_COUCHBASE_LOCAL
void php_couchbase_res_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	php_couchbase_res *couchbase_res = (php_couchbase_res *)rsrc->ptr;
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

PHP_COUCHBASE_LOCAL
void php_couchbase_pres_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	php_couchbase_res *couchbase_res = (php_couchbase_res *)rsrc->ptr;
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
static void php_couchbase_error_callback(lcb_t handle, lcb_error_t error, const char *errinfo) /* {{{ */
{
	php_couchbase_res *res = (php_couchbase_res *)lcb_get_cookie(handle);
	/**
	 * @FIXME: when connect to a non-couchbase-server port (but the socket is valid)
	 * like a apache server, process will be hanged by event_loop
	 */
	if (res && res->seqno < 0) {
		pcbc_stop_loop(res);
	}
}
/* }}} */

/* {{{ static void php_couchbase_stat_callback(...) */
static void php_couchbase_stat_callback(lcb_t handle,
										const void *cookie,
										lcb_error_t error,
										const lcb_server_stat_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	const void *key = resp->v.v0.key;
	char *string_key;
	size_t nkey = resp->v.v0.nkey;
	const void *bytes = resp->v.v0.bytes;

	php_ignore_value(handle);

	ctx->res->rc = error;
	if (LCB_SUCCESS != error || nkey == 0) {
		--ctx->res->seqno;
		pcbc_stop_loop(ctx->res);
		return;
	} else if (nkey > 0) {
		zval *node;
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

		string_key = emalloc(nkey + 1);
		memcpy(string_key, key, nkey);
		string_key[nkey] = '\0';

		add_assoc_string(node, string_key, (char *)bytes, 1);

		efree(string_key);
	}
}
/* }}} */

/* {{{ static void php_couchbase_version_callback(...) */
static void
php_couchbase_version_callback(lcb_t handle,
							   const void *cookie,
							   lcb_error_t error,
							   const lcb_server_version_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	const char *version_string = resp->v.v0.vstring;
	lcb_size_t nversion_string = resp->v.v0.nvstring;
	php_ignore_value(handle);

	ctx->res->rc = error;
	if (LCB_SUCCESS != error || nversion_string == 0 || server_endpoint == NULL) {
		--ctx->res->seqno;
		pcbc_stop_loop(ctx->res);
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

PHP_COUCHBASE_LOCAL
void php_couchbase_setup_callbacks(lcb_t handle)
{
	php_couchbase_callbacks_arithmetic_init(handle);
	php_couchbase_callbacks_get_init(handle);
	php_couchbase_callbacks_store_init(handle);
	php_couchbase_callbacks_remove_init(handle);
	php_couchbase_callbacks_touch_init(handle);
	php_couchbase_callbacks_observe_init(handle);
	php_couchbase_callbacks_view_init(handle);
	php_couchbase_callbacks_flush_init(handle);

	php_ignore_value(lcb_set_stat_callback(handle, php_couchbase_stat_callback));
	php_ignore_value(lcb_set_version_callback(handle, php_couchbase_version_callback));
	php_ignore_value(lcb_set_error_callback(handle, php_couchbase_error_callback));
}

PHP_COUCHBASE_LOCAL
long pcbc_check_expiry(long expiry)
{
	if (expiry < 0) {
		php_error(E_RECOVERABLE_ERROR, "Expiry must not be negative (%ld given).", expiry);
	}
	return expiry;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_server_stats_cmd_t cmd;
			lcb_server_stats_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			retval = lcb_server_stats(couchbase_res->handle, (const void *)ctx,
									  1, (const lcb_server_stats_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Failed to schedule stat request: %s [%d]",
							 lcb_strerror(couchbase_res->handle, retval),
							 retval);
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS != ctx->res->rc) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Failed to stat: (%d): %s",
							 ctx->res->rc,
							 lcb_strerror(couchbase_res->handle, ctx->res->rc));
			efree(ctx);
			RETURN_FALSE;
		}
		efree(ctx);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_version_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_server_version_cmd_t cmd;
			lcb_server_version_cmd_t *commands[] = { &cmd };
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
		pcbc_start_loop(couchbase_res);
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


PHP_COUCHBASE_LOCAL
void php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	long option;
	zval *value;
	php_couchbase_res *couchbase_res;
	int argflags =
		(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
		PHP_COUCHBASE_ARG_F_NOCONN | PHP_COUCHBASE_ARG_F_ASYNC;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "lz", &option, &value);

	switch (option) {
	case COUCHBASE_OPT_SERIALIZER: {
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
	case COUCHBASE_OPT_PREFIX_KEY: {
		convert_to_string_ex(&value);
		if (couchbase_res->prefix_key) {
			efree(couchbase_res->prefix_key);
		}
		couchbase_res->prefix_key = estrndup(Z_STRVAL_P(value), Z_STRLEN_P(value));
		couchbase_res->prefix_key_len = Z_STRLEN_P(value);
	}
	break;
	case COUCHBASE_OPT_COMPRESSION: {
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
	case COUCHBASE_OPT_IGNOREFLAGS: {
		convert_to_long_ex(&value);
		couchbase_res->ignoreflags = Z_LVAL_P(value);
		break;
	}
	case COUCHBASE_OPT_VOPTS_PASSTHROUGH: {
		convert_to_long_ex(&value);
		couchbase_res->viewopts_passthrough = Z_LVAL_P(value);
		break;
	}
	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
		break;
	}
	RETURN_FALSE;
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	long option;
	php_couchbase_res *couchbase_res;
	int argflags =
		(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
		PHP_COUCHBASE_ARG_F_ONLYVALID;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "l", &option);

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
	case COUCHBASE_OPT_IGNOREFLAGS:
		RETURN_LONG(couchbase_res->ignoreflags);
		break;
	case COUCHBASE_OPT_VOPTS_PASSTHROUGH:
		RETURN_LONG(couchbase_res->viewopts_passthrough);
		break;
	default:
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "unknown option type: %ld", option);
		break;
	}
	RETURN_FALSE;
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	php_couchbase_res *couchbase_res;
	char *str;
	int str_len;
	int argflags = (oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL)
				   | PHP_COUCHBASE_ARG_F_ONLYVALID;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");
	str_len = spprintf(&str, 0, "%s", lcb_strerror(couchbase_res->handle, couchbase_res->rc));
	RETURN_STRINGL(str, str_len, 0);
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
