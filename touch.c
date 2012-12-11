#include "internal.h"

static void php_couchbase_touch_callback(lcb_t handle,
										 const void *cookie,
										 lcb_error_t error,
										 const lcb_touch_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const char *key = (char *)resp->v.v0.key;
	lcb_size_t nkey = resp->v.v0.nkey;
	char *string_key;
	php_ignore_value(handle);

	// TODO: is cas needed? existing php docs don't say anything about
	// it being used, but it's in the resp struct...  lcb_cas_t cas =
	// resp->v.v0.cas;

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
	}

	ctx->res->rc = error;

	if (LCB_SUCCESS != error || key == NULL || nkey == 0) {
		return;
	} else if (nkey > 0) {
		if (IS_ARRAY == Z_TYPE_P(ctx->rv)) {
			// set (key name => true) within return value associative
			// array (we did touch it)
			string_key = emalloc(nkey + 1);
			memcpy(string_key, key, nkey);
			string_key[nkey] = '\0';

			add_assoc_bool(ctx->rv, string_key, (zend_bool)1);

			efree(string_key);
		} else {
			// set return val to true (we touched the one thing we set
			// out to touch)
			ZVAL_BOOL(ctx->rv, 1);
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo)
{
	char *single_key = NULL;   /* for a single key */
	long single_nkey = 0;   /* (size of key string) */
	char **multi_keys = NULL; /* for an array of keys */
	long keycount = 0;  /* (size of array of key strings, and of array of key strings' sizes) */
	long *keyslens = NULL; /* (array of sizes of key strings) */
	lcb_time_t exp = {0}; /* how long to set expiry. */
	long expiry; /* used for parameter passing */
	/* note that by server's behavior, anything longer than 30 days (60*60*24*30) is an epoch time to expire at */
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;
	int argflags;

	/* parameter handling and return_value setup: */
	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	if (multi) {
		zval *arr_keys;
		zval **ppzval;
		int i;

		PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
								 "al", &arr_keys, &expiry);

		keycount = zend_hash_num_elements(Z_ARRVAL_P(arr_keys));
		multi_keys = ecalloc(keycount, sizeof(char *));
		keyslens = ecalloc(keycount, sizeof(long));

		array_init(return_value);

		for (i = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(arr_keys));
				zend_hash_has_more_elements(Z_ARRVAL_P(arr_keys)) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(arr_keys)), i++) {
			if (zend_hash_get_current_data(Z_ARRVAL_P(arr_keys), (void **)&ppzval) == FAILURE) {
				keycount--;
				continue;
			}

			if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
				convert_to_string_ex(ppzval);
			}

			if (!Z_STRLEN_PP(ppzval)) {
				keycount--;
				continue;
			}

			if (couchbase_res->prefix_key_len) {
				keyslens[i] = spprintf(&(multi_keys[i]), 0, "%s_%s", couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
			} else {
				multi_keys[i] = Z_STRVAL_PP(ppzval);
				keyslens[i] = Z_STRLEN_PP(ppzval);
			}

			/* set keyname => false in the return array (will get set
			   to true in the touch callback when/if keyname seen) */
			add_assoc_bool(return_value, multi_keys[i], (zend_bool)0);
		}

		if (!keycount) {
			efree(multi_keys);
			efree(keyslens);
			return;
		}
	} else { /* single-valued */

		PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
								 "sl", &single_key, &single_nkey, &expiry);

		if (!single_nkey) {
			return;
		}

		keycount = 1;
		if (couchbase_res->prefix_key_len) {
			single_nkey = spprintf(&single_key, 0, "%s_%s", couchbase_res->prefix_key, single_key);
		}
		multi_keys = &single_key;
		keyslens = &single_nkey;

		/* set return value false, will get set to true in the touch
		   callback when/if the operation succeeds */
		ZVAL_FALSE(return_value);
	}

	/* main action */
	{
		lcb_touch_cmd_t **commands = ecalloc(keycount, sizeof(lcb_touch_cmd_t *));
		lcb_error_t retval;
		int ii;

		if (expiry) {
			exp = pcbc_check_expiry(expiry);
		}

		for (ii = 0; ii < keycount; ++ii) {
			lcb_touch_cmd_t *cmd = ecalloc(1, sizeof(lcb_touch_cmd_t));
			cmd->version = 0;
			cmd->v.v0.key = multi_keys[ii];
			cmd->v.v0.nkey = keyslens[ii];
			cmd->v.v0.exptime = exp; /* note: this assumes sizeof(long) == sizeof(lcb_time_t) */
			commands[ii] = cmd;
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		retval = lcb_touch(couchbase_res->handle, ctx, keycount, (const lcb_touch_cmd_t * const *)commands);
		for (ii = 0; ii < keycount; ++ii) {
			efree(commands[ii]);
		}
		efree(commands);

		if (LCB_SUCCESS != retval) {
			if (couchbase_res->prefix_key_len) {
				int i;
				for (i = 0; i < keycount; i++) {
					efree(multi_keys[i]);
				}
			}

			if (multi) {
				efree(multi_keys);
				efree(keyslens);
				zval_dtor(return_value);
			}

			efree(ctx);

			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_lcb_exception,
								   "Failed to schedule touch request: %s",
								   lcb_strerror(couchbase_res->handle, retval));
			return;
		}

		couchbase_res->seqno += keycount;
		pcbc_start_loop(couchbase_res);
		if (ctx->res->rc != LCB_SUCCESS) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_lcb_exception,
								   "Failed touch request: %s",
								   lcb_strerror(couchbase_res->handle,
												ctx->res->rc));
		}

		efree(ctx);
		if (couchbase_res->prefix_key_len) {
			int i;
			for (i = 0; i < keycount; i++) {
				efree(multi_keys[i]);
			}
		}

		if (multi) {
			efree(multi_keys);
			efree(keyslens);
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_touch_init(lcb_t handle)
{
	lcb_set_touch_callback(handle, php_couchbase_touch_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
