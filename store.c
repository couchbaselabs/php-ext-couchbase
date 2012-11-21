#include "internal.h"

/* {{{ static void php_couchbase_storage_callback(...)
 */
static void
php_couchbase_store_callback(lcb_t instance,
                             const void *cookie,
                             lcb_storage_t operation,
                             lcb_error_t error,
                             const lcb_store_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const void *key;
	size_t nkey;
	uint64_t cas;
	php_ignore_value(instance);
	php_ignore_value(operation);

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
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

PHP_COUCHBASE_LOCAL
void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, lcb_storage_t op, int multi, int oo) /* {{{ */
{
	zval *akc = NULL, *adurability = NULL;
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;
	time_t exp = {0};
	unsigned int flags = 0;
	char *payload, *cas = NULL;
	size_t payload_len = 0;
	unsigned long long cas_v = 0;
	long expire = 0, cas_len = 0;
	char *key = NULL;

	if (!multi) {
		char *key = NULL;
		zval *value;
		long klen = 0;
		PHP_COUCHBASE_GET_PARAMS(couchbase_res,
				oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL,
						"sz|lsa",
						&key, &klen,
						&value,
						&expire,
						&cas, &cas_len, &adurability);

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
			exp = pcbc_check_expiry(expire);
		}

		if (cas) {
			cas_v = strtoull(cas, 0, 10);
		}

		{
			lcb_store_cmd_t cmd;
			lcb_store_cmd_t *commands[] = { &cmd };
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

	} else { /* multi */
		zval *akeys, **ppzval;
		char *key = NULL;
		uint klen = 0;
		ulong idx;
		int key_type, nkey = 0;

		PHP_COUCHBASE_GET_PARAMS(couchbase_res,
				oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL,
				"a|la",
				&akeys, &expire, &adurability);

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv	 = return_value;
		array_init(ctx->rv);

		if (expire) {
			exp = pcbc_check_expiry(expire);
		}

		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
		        zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
		        zend_hash_move_forward(Z_ARRVAL_P(akeys))) {
			if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void **)&ppzval) == FAILURE) {
				continue;
			}
			switch ((key_type = zend_hash_get_current_key(Z_ARRVAL_P(akeys), &key, &idx, 0))) {
			case HASH_KEY_IS_LONG:
				spprintf(&key, 0, "%ld", idx);
				break;
			case HASH_KEY_IS_STRING:
				break;
			default:
				continue;
			}

			if (!(klen = strlen(key))) {
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
				lcb_store_cmd_t *commands[] = { &cmd };
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
		pcbc_start_loop(couchbase_res);
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

		/* If we have a durability spec, after the commands have been issued (and callbacks returned), try to
		 * fulfill that spec by using polling observe internal:
		 */
		if (adurability != NULL) {
			array_init(akc);

			if (IS_ARRAY == Z_TYPE_P(return_value)) { /* multi */
				ulong curr_idx;
				char *curr_key;
				uint curr_key_len;
				zval **curr_cas;

				for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(return_value));
				        zend_hash_has_more_elements(Z_ARRVAL_P(return_value)) == SUCCESS;
				        zend_hash_move_forward(Z_ARRVAL_P(return_value))) {
					zend_hash_get_current_key_ex(Z_ARRVAL_P(return_value), (char **)&curr_key, &curr_key_len, &curr_idx, 0, NULL);
					zend_hash_get_current_data(Z_ARRVAL_P(return_value), (void **)&curr_cas);
					if (Z_BVAL_PP(curr_cas)) {
						add_assoc_long(akc, curr_key, Z_LVAL_PP(curr_cas));
					}
				}
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(return_value));
			} else { /* not multi */
				if (Z_BVAL_P(return_value)) { /* it claims to have stored */
					add_assoc_long(akc, key, Z_LVAL_P(return_value));
				}
			}

			ctx->cas = akc;

			observe_polling_internal(ctx, adurability, 0);
		}

		efree(ctx);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	zval *value;
	time_t exp = {0};
	unsigned int flags = 0;
	size_t payload_len = 0;
	unsigned long long cas_v = 0;
	char *key, *payload, *cas = NULL;
	long klen = 0, expire = 0, cas_len = 0;
	php_couchbase_res *couchbase_res;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res,
			oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL,
					"ssz|l",
					&cas, &cas_len,
					&key, &klen,
					&value, &expire);

	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		if (expire) {
			exp = pcbc_check_expiry(expire);
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
			lcb_store_cmd_t *commands[] = { &cmd };
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
		pcbc_start_loop(couchbase_res);
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

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_store_init(lcb_t handle)
{
	lcb_set_store_callback(handle, php_couchbase_store_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
