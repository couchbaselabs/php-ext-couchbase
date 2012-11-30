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
  | Author: Trond Norbye	   <trond.norbye@gmail.com>					 |
  +----------------------------------------------------------------------+
*/
#include "internal.h"

static void php_couchbase_version_callback(lcb_t handle,
										   const void *cookie,
										   lcb_error_t error,
										   const lcb_server_version_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *server_endpoint = resp->v.v0.server_endpoint;
	const char *version_string = resp->v.v0.vstring;
	lcb_size_t nversion_string = resp->v.v0.nvstring;
	php_ignore_value(handle);

	if (error != LCB_SUCCESS) {
		ctx->res->rc = error;
		return;
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}

	if (nversion_string > 0) {
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

PHP_COUCHBASE_LOCAL
void php_couchbase_version_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	lcb_server_version_cmd_t cmd;
	const lcb_server_version_cmd_t *const commands[] = { &cmd };
	lcb_error_t retval;
	php_couchbase_ctx *ctx;
	php_couchbase_res *couchbase_res;
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
	retval = lcb_server_versions(instance, (const void *)ctx, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS) {
		retval = couchbase_res->rc;
	}
	couchbase_res->rc = retval;

	efree(ctx);
	if (retval != LCB_SUCCESS) {
		char errmsg[256];
		snprintf(errmsg, sizeof(errmsg), "Failed to fetch server version: %s",
				 lcb_strerror(instance, retval));
		if (oo) {
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
			return ;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
			RETURN_FALSE;
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_version_init(lcb_t handle)
{
	lcb_set_version_callback(handle, php_couchbase_version_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
