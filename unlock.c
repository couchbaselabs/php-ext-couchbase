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

static void php_couchbase_unlock_callback(lcb_t instance,
										  const void *cookie,
										  lcb_error_t error,
										  const lcb_unlock_resp_t *resp)
{
	lcb_error_t *err = (lcb_error_t *)cookie;
	*err = error;
}

static lcb_error_t do_unlock(lcb_t instance, const void *key, uint16_t klen,
							 lcb_cas_t cas)
{
	lcb_error_t error;
	lcb_unlock_cmd_t cmd;
	const lcb_unlock_cmd_t *const commands[] = { &cmd };
	lcb_error_t retval;

	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = klen;
	cmd.v.v0.cas = cas;

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_unlock(instance, &error, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	return (retval == LCB_SUCCESS) ? error : retval;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_unlock_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	char *key;
	char *cas = NULL;
	long klen = 0;
	long cas_len = 0;
	lcb_cas_t cas_v = 0;
	php_couchbase_res *couchbase_res;
	lcb_error_t retval;

	int arg = (oo) ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res,  arg,
							 "ss", &key, &klen, &cas, &cas_len);

	if (klen == 0) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "Failed to schedule set request: Empty key");
		return ;
	}

	if (cas_len == 0) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "No CAS specified: Empty cas");
		return ;
	}

	cas_v = (lcb_cas_t)strtoull(cas, 0, 10);
	retval = do_unlock(couchbase_res->handle, key, klen, cas_v);
	couchbase_res->rc = retval;

	if (retval == LCB_SUCCESS) {
		RETVAL_TRUE;
	} else {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to unlock the document: %s",
							   lcb_strerror(couchbase_res->handle, retval));
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_unlock_init(lcb_t handle)
{
	lcb_set_unlock_callback(handle, php_couchbase_unlock_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
