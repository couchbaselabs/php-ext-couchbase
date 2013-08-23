/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2013 Couchbase, Inc.                                       |
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

struct touch_cookie {
	char *key;
	size_t nkey;
	lcb_error_t error;
	lcb_cas_t cas;
};

static void single_touch_callback(lcb_t handle,
								  const void *cookie,
								  lcb_error_t error,
								  const lcb_touch_resp_t *resp)
{
	struct touch_cookie *tc = (void *)cookie;
	tc->error = error;
	if (resp->version != 0) {
		tc->error = LCB_ERROR;
	} else {
		tc->cas = resp->v.v0.cas;
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	struct touch_cookie cookie;
	char *key = NULL;
	long nkey = 0;
	lcb_time_t exp = 0;
	long expiry;
	php_couchbase_res *couchbase_res;
	int argflags;
	lcb_t instance;
	lcb_error_t retval;
	lcb_touch_cmd_t cmd;
	const lcb_touch_cmd_t *const commands[] = { &cmd };

	/* parameter handling and return_value setup: */
	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "sl", &key, &nkey, &expiry);

	if (pcbc_check_expiry(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
						  expiry, &exp) == -1) {
		/* Incorrect expiry time */
		return;
	}

	if (!nkey) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "No key specified: Empty key");
		return;
	}

	if (couchbase_res->prefix_key_len) {
		nkey = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = nkey;
	cmd.v.v0.exptime = exp;

	instance = couchbase_res->handle;
	cookie.error = LCB_ERROR;
	lcb_set_touch_callback(instance, single_touch_callback);
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_touch(instance, &cookie, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS) {
		retval = cookie.error;
	}
	couchbase_res->rc = retval;

	if (couchbase_res->prefix_key_len) {
		efree(key);
	}

	if (retval != LCB_SUCCESS) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to touch key: %s",
							   lcb_strerror(instance, retval));
		return;
	} else {
		Z_TYPE_P(return_value) = IS_STRING;
		Z_STRLEN_P(return_value) = spprintf(&(Z_STRVAL_P(return_value)), 0,
											"%"PRIu64, cookie.cas);
	}
}

struct multi_touch_cookie {
	struct touch_cookie *keys;
	int nkeys;
	lcb_error_t error;
};

static void multi_touch_callback(lcb_t handle,
								 const void *cookie,
								 lcb_error_t error,
								 const lcb_touch_resp_t *resp)
{
	struct multi_touch_cookie *mtc = (void *)cookie;
	int ii;

	if (resp->version != 0) {
		mtc->error = LCB_ERROR;
	}

	if (mtc->error != LCB_SUCCESS) {
		return;
	}

	/* Locate the correct request */
	for (ii = 0; ii < mtc->nkeys; ++ii) {
		if (resp->v.v0.nkey == mtc->keys[ii].nkey && !memcmp(mtc->keys[ii].key,
															 resp->v.v0.key,
															 resp->v.v0.nkey)) {
			mtc->keys[ii].cas = resp->v.v0.cas;
			mtc->keys[ii].error = error;
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_touch_multi_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	int argflags;
	int idx = 0;
	int ii;
	lcb_error_t retval;
	lcb_t instance;
	lcb_time_t exp = 0;
	lcb_touch_cmd_t **commands;
	lcb_touch_cmd_t *cmd;
	long expiry;
	php_couchbase_res *couchbase_res;
	struct multi_touch_cookie cookie;
	zval **ppzval;
	zval *arr_keys;

	/* parameter handling and return_value setup: */
	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "al", &arr_keys, &expiry);

	if (pcbc_check_expiry(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
						  expiry, &exp) == -1) {
		/* Incorrect expiry time */
		return;
	}

	instance = couchbase_res->handle;
	memset(&cookie, 0, sizeof(cookie));
	cookie.nkeys = zend_hash_num_elements(Z_ARRVAL_P(arr_keys));
	cookie.keys = ecalloc(cookie.nkeys, sizeof(struct touch_cookie));

	for (ii = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr_keys));
			zend_hash_has_more_elements(Z_ARRVAL_P(arr_keys)) == SUCCESS;
			zend_hash_move_forward(Z_ARRVAL_P(arr_keys)), ii++) {
		if (zend_hash_get_current_data(Z_ARRVAL_P(arr_keys),
									   (void **)&ppzval) == FAILURE) {
			continue;
		}

		if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
			convert_to_string_ex(ppzval);
		}

		if (!Z_STRLEN_PP(ppzval)) {
			continue;
		}

		if (couchbase_res->prefix_key_len) {
			cookie.keys[idx].nkey = spprintf(&(cookie.keys[idx].key), 0,
											 "%s_%s",
											 couchbase_res->prefix_key,
											 Z_STRVAL_PP(ppzval));
		} else {
			cookie.keys[idx].key = Z_STRVAL_PP(ppzval);
			cookie.keys[idx].nkey = Z_STRLEN_PP(ppzval);
		}
		++idx;
	}

	if (idx == 0) {
		efree(cookie.keys);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "No keys specified");
		return;
	}

	cmd = ecalloc(idx, sizeof(lcb_touch_cmd_t));
	commands = ecalloc(idx, sizeof(lcb_touch_cmd_t *));
	for (ii = 0; ii < idx; ++ii) {
		cmd[ii].v.v0.key = cookie.keys[ii].key;
		cmd[ii].v.v0.nkey = cookie.keys[ii].nkey;
		cmd[ii].v.v0.exptime = exp;
		commands[ii] = cmd + ii;
	}

	lcb_set_touch_callback(instance, multi_touch_callback);
	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_touch(instance, &cookie, idx,
					   (const lcb_touch_cmd_t * const *)commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS) {
		retval = cookie.error;
	}

	couchbase_res->rc = retval;
	if (retval == LCB_SUCCESS) {
		/* Time to build up the array with the keys we found etc */
		array_init(return_value);
		for (ii = 0; ii < idx; ++ii) {
			char *k = cookie.keys[ii].key;
			if (cookie.keys[ii].error == LCB_SUCCESS) {
				char cas[80];
				snprintf(cas, sizeof(cas), "%"PRIu64,
						 (unsigned long long)cookie.keys[ii].cas);
				add_assoc_string(return_value, k, cas, 1);
			} else {
				add_assoc_bool(return_value, k, (zend_bool)0);
			}
		}
	} else {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to touch key: %s",
							   lcb_strerror(instance, retval));
	}

	/* Release allocated memory */
	efree(cmd);
	efree(commands);
	if (couchbase_res->prefix_key_len) {
		for (ii = 0; ii < idx; ++ii) {
			efree(cookie.keys[ii].key);
		}
	}
	efree(cookie.keys);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
