/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.                                       |
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
*/
#include "internal.h"

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

	if (resp->version != 0) {
		error = LCB_ERROR;
	}

	if (error != LCB_SUCCESS) {
		/* preserve the first error */
		ctx->res->rc = error;
		return ;
	}

	if (nkey > 0) {
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

PHP_COUCHBASE_LOCAL
void php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	php_couchbase_res *couchbase_res;
	lcb_error_t retval;
	php_couchbase_ctx *ctx;
	lcb_server_stats_cmd_t cmd;
	const lcb_server_stats_cmd_t *const commands[] = { &cmd };
	lcb_t instance;
	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	ctx = ecalloc(1, sizeof(php_couchbase_ctx));
	ctx->res = couchbase_res;
	ctx->rv = return_value;
	couchbase_res->rc = LCB_SUCCESS;
	instance = couchbase_res->handle;

	memset(&cmd, 0, sizeof(cmd));

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_server_stats(instance, (const void *)ctx, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);
	if (retval == LCB_SUCCESS) {
		retval = couchbase_res->rc;
	} else {
		couchbase_res->rc = retval;
	}

	efree(ctx);

	if (retval != LCB_SUCCESS) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception, "Failed to stat: %s",
							   lcb_strerror(instance, retval));
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_stat_init(lcb_t handle)
{
	lcb_set_stat_callback(handle, php_couchbase_stat_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
