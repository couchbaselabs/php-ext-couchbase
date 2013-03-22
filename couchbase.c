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
		free(couchbase_res->bucket);
		if (couchbase_res->prefix_key) {
			free((void *)couchbase_res->prefix_key);
		}
		free(couchbase_res);
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
		free(couchbase_res->bucket);
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

PHP_COUCHBASE_LOCAL
void php_couchbase_setup_callbacks(lcb_t handle)
{
	php_couchbase_callbacks_arithmetic_init(handle);
	php_couchbase_callbacks_get_init(handle);
	php_couchbase_callbacks_store_init(handle);
	php_couchbase_callbacks_remove_init(handle);
	php_couchbase_callbacks_observe_init(handle);
	php_couchbase_callbacks_view_init(handle);
	php_couchbase_callbacks_stat_init(handle);
	php_couchbase_callbacks_version_init(handle);
	php_couchbase_callbacks_unlock_init(handle);

	lcb_set_error_callback(handle, php_couchbase_error_callback);
}

PHP_COUCHBASE_LOCAL
int pcbc_check_expiry(INTERNAL_FUNCTION_PARAMETERS, int oo, long expiry, long *out)
{
	if (expiry < 0) {

		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_arguments_exception,
							   "Expiry must not be negative (%ld given).",
							   expiry);

		/* php_error(E_RECOVERABLE_ERROR, "Expiry must not be negative (%ld given).", expiry); */
		return -1;
	}

	*out = (time_t)expiry;
	return 0;
}

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
