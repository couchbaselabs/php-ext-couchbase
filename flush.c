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

static void php_couchbase_flush_callback(lcb_t handle,
										 const void *cookie,
										 lcb_error_t error,
										 const lcb_flush_resp_t *resp)
{
	if (error != LCB_SUCCESS) {
		*((lcb_error_t *)cookie) = error;
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	php_couchbase_res *couchbase_res;
	lcb_error_t retval;
	lcb_error_t cberr = LCB_SUCCESS;
	php_couchbase_ctx *ctx;
	lcb_flush_cmd_t cmd;
	const lcb_flush_cmd_t *const commands[] = { &cmd };
	lcb_t instance;

	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	instance = couchbase_res->handle;

	memset(&cmd, 0, sizeof(cmd));
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_flush(instance, (const void *)&cberr, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS) {
		retval = cberr;
	}
	couchbase_res->rc = retval;

	if (retval != LCB_SUCCESS) {
		char errmsg[256];
		sprintf(errmsg, "Failed to flush bucket: %s",
				lcb_strerror(instance, retval));
		if (oo) {
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
			return ;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
			RETURN_FALSE;
		}
	}

	RETURN_TRUE;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_flush_init(lcb_t handle)
{
	lcb_set_flush_callback(handle, php_couchbase_flush_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
