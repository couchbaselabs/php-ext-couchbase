#include "internal.h"

/* {{{ static void php_couchbase_storage_callback(...)
 */
static void php_couchbase_store_callback(lcb_t instance,
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
			zend_hash_update(Z_ARRVAL_P(ctx->rv), string_key, nkey + 1,
							 (void **)&rv, sizeof(zval *), NULL);
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
		zend_hash_update(Z_ARRVAL_P(ctx->rv), string_key, nkey + 1,
						 (void **)&rv, sizeof(zval *), NULL);
		efree(string_key);
	} else {
		Z_TYPE_P(ctx->rv) = IS_STRING;
		Z_STRLEN_P(ctx->rv) = spprintf(&(Z_STRVAL_P(ctx->rv)), 0, "%llu", cas);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, lcb_storage_t op, int multi) /* {{{ */
{
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
								 PHP_COUCHBASE_ARG_F_FUNCTIONAL,
								 "sz|ls",
								 &key, &klen,
								 &value,
								 &expire,
								 &cas, &cas_len);

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
								 PHP_COUCHBASE_ARG_F_FUNCTIONAL,
								 "a|l",
								 &akeys, &expire);

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
	long replicate_to = 0;
	long persist_to = 0;
	lcb_error_t retval;
	php_couchbase_ctx *ctx;
	lcb_store_cmd_t cmd;
	const lcb_store_cmd_t *const commands[] = { &cmd };
	int argf = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argf, "ssz|lll", &cas, &cas_len,
							 &key, &klen, &value, &expire, &persist_to,
							 &replicate_to);

	if (validate_simple_observe_clause(couchbase_res->handle,
									   persist_to,
									   replicate_to TSRMLS_CC) == -1) {
		/* Exception already thrown */
		return;
	}

	if (klen == 0) {
		if (oo) {
			zend_throw_exception(cb_illegal_key_exception,
								 "Failed to schedule set request: Empty key",
								 0 TSRMLS_CC);
			return;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Failed to schedule set request: Empty key");
			RETURN_FALSE;
		}
	}

	if (cas_len == 0) {
		if (oo) {
			zend_throw_exception(cb_exception,
								 "Invalid cas specified",
								 0 TSRMLS_CC);
			return;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "Invalid cas specified");
			RETURN_FALSE;
		}
	}

	ctx = ecalloc(1, sizeof(php_couchbase_ctx));
	ctx->res = couchbase_res;
	ctx->rv = return_value;

	if (expire) {
		exp = pcbc_check_expiry(expire);
	}

	if (cas) {
		cas_v = strtoull(cas, 0, 10);
	}

	payload = php_couchbase_zval_to_payload(value, &payload_len, &flags,
											couchbase_res->serializer,
											couchbase_res->compressor
											TSRMLS_CC);
	if (payload == NULL) {
		RETURN_FALSE;
	}

	if (couchbase_res->prefix_key_len) {
		klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.operation = LCB_SET;
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = klen;
	cmd.v.v0.bytes = payload;
	cmd.v.v0.nbytes = payload_len;
	cmd.v.v0.flags = flags;
	cmd.v.v0.exptime = exp;
	cmd.v.v0.cas = (uint64_t)cas_v;

	retval = lcb_store(couchbase_res->handle, ctx, 1, commands);

	efree(payload);
	if (couchbase_res->prefix_key_len) {
		efree(key);
	}

	if (retval != LCB_SUCCESS) {
		char errmsg[256];
		efree(ctx);
		sprintf(errmsg, "Failed to schedule cas request: %s",
				lcb_strerror(couchbase_res->handle, retval));

		if (oo) {
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
			return;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
			RETURN_FALSE;
		}
	}

	++couchbase_res->seqno;
	pcbc_start_loop(couchbase_res);
	zval_dtor(return_value);

	if (LCB_SUCCESS == ctx->res->rc) {
		ZVAL_TRUE(return_value);
	} else if (LCB_KEY_EEXISTS == ctx->res->rc) {
		ZVAL_FALSE(return_value);
	} else {
		char errmsg[256];
		sprintf(errmsg, "Failed to store a value to server: %s",
				lcb_strerror(couchbase_res->handle, ctx->res->rc));
		if (oo) {
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
		} else {
			ZVAL_FALSE(return_value);
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
		}
	}
	efree(ctx);
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_store_init(lcb_t handle)
{
	lcb_set_store_callback(handle, php_couchbase_store_callback);
}

PHP_COUCHBASE_LOCAL
void php_couchbase_store_impl_oo(INTERNAL_FUNCTION_PARAMETERS, lcb_storage_t op)
{
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;
	time_t exp = {0};
	unsigned int flags = 0;
	char *cas = NULL;
	char *payload;
	size_t payload_len = 0;
	unsigned long long cas_v = 0;
	long expire = 0;
	long replicate_to = 0;
	long persist_to = 0;
	long cas_len = 0;
	char *key = NULL;
	zval *value;
	long klen = 0;
	lcb_store_cmd_t cmd;
	const lcb_store_cmd_t *const commands[] = { &cmd };

	if (op == LCB_ADD) {
		PHP_COUCHBASE_GET_PARAMS(couchbase_res, PHP_COUCHBASE_ARG_F_OO,
								 "sz|lll", &key, &klen, &value, &expire,
								 &persist_to, &replicate_to);
	} else {
		PHP_COUCHBASE_GET_PARAMS(couchbase_res, PHP_COUCHBASE_ARG_F_OO,
								 "sz|lsll", &key, &klen, &value, &expire,
								 &cas, &cas_len, &persist_to, &replicate_to);
	}

	if (validate_simple_observe_clause(couchbase_res->handle,
									   persist_to,
									   replicate_to TSRMLS_CC) == -1) {
		/* Exception already thrown */
		return;
	}

	if (!klen) {
		zend_throw_exception(cb_illegal_key_exception,
							 "Failed to schedule set request: Empty key",
							 0 TSRMLS_CC);
		return ;
	}

	payload = php_couchbase_zval_to_payload(value, &payload_len, &flags,
											couchbase_res->serializer,
											couchbase_res->compressor
											TSRMLS_CC);
	if (payload == NULL) {
		/* ?? I guess we should throw an exception here? */
		RETURN_FALSE;
	}

	if (couchbase_res->prefix_key_len) {
		klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
	}

	ctx = ecalloc(1, sizeof(php_couchbase_ctx));
	ctx->res = couchbase_res;
	ctx->rv = return_value;
	couchbase_res->seqno += 1;

	if (expire) {
		exp = pcbc_check_expiry(expire);
	}

	if (cas) {
		cas_v = strtoull(cas, 0, 10);
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.operation = op;
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = klen;
	cmd.v.v0.bytes = payload;
	cmd.v.v0.nbytes = payload_len;
	cmd.v.v0.flags = flags;
	cmd.v.v0.exptime = exp;
	cmd.v.v0.cas = (uint64_t)cas_v;

	retval = lcb_store(couchbase_res->handle, ctx, 1, commands);

	efree(payload);
	if (couchbase_res->prefix_key_len) {
		efree(key);
	}

	if (retval != LCB_SUCCESS) {
		char errmsg[256];
		efree(ctx);
		sprintf(errmsg, "Failed to schedule set request: %s",
				lcb_strerror(couchbase_res->handle, retval));
		zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
		return ;
	}

	pcbc_start_loop(couchbase_res);
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
		default: {
			char errmsg[256];
			sprintf(errmsg, "Failed to store value to server: %s",
					lcb_strerror(couchbase_res->handle, ctx->res->rc));
			zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
		}

		break;
		}
	} else {
		/*
		 * The item was stored successfully. Did the user want to wait until
		 * it was persisted/replicated?
		 */
		if (persist_to != 0 || replicate_to != 0) {
			struct observe_entry entry;
			memset(&entry, 0, sizeof(entry));
			entry.key = key;
			entry.nkey = klen;
			entry.cas = strtoull(Z_STRVAL_P(return_value), 0, 10);

			retval = simple_observe(couchbase_res->handle, &entry, 1,
									persist_to, replicate_to);
			couchbase_res->rc = retval;
			if (retval != LCB_SUCCESS) {
				if (retval == LCB_ETIMEDOUT) {
					zend_throw_exception(cb_timeout_exception,
										 "Timed out waiting for the objects to persist",
										 0 TSRMLS_CC);
				} else {
					char errmsg[512];
					snprintf(errmsg, sizeof(errmsg), "observe failed for: %s",
							 klen, key, lcb_strerror(couchbase_res->handle,
													 retval));
					zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
				}
			} else {
				/* @todo checkfor timeout!!! */
				if (entry.mutated) {
					zend_throw_exception(cb_key_mutated_exception,
										 "The document was mutated",
										 0 TSRMLS_CC);
				}
			}
		}
	}
	efree(ctx);
}
/* }}} */

static void release_entry_array(struct observe_entry *entries,  int nent)
{
	int ii;
	for (ii = 0; ii < nent; ++ii) {
		efree(entries[ii].key);
	}
	efree(entries);
}


PHP_COUCHBASE_LOCAL
void php_couchbase_store_multi_impl_oo(INTERNAL_FUNCTION_PARAMETERS)
{
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;
	time_t exp = {0};
	long expire = 0;
	zval *akeys;
	long persist_to = 0;
	long replicate_to = 0;
	struct observe_entry *entries;
	int numkeys;
	lcb_store_cmd_t *cmds;
	lcb_store_cmd_t **commands;
	int ii;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, PHP_COUCHBASE_ARG_F_OO, "a|lll",
							 &akeys, &expire, &persist_to, &replicate_to);

	if (validate_simple_observe_clause(couchbase_res->handle,
									   persist_to,
									   replicate_to TSRMLS_CC) == -1) {
		/* Exception already thrown */
		return;
	}

	numkeys = zend_hash_num_elements(Z_ARRVAL_P(akeys));
	if (numkeys == 0) {
		zend_throw_exception(cb_illegal_key_exception,
							 "No items specified",
							 0 TSRMLS_CC);
		return ;
	}

	entries = ecalloc(numkeys, sizeof(struct observe_entry));
	commands = ecalloc(numkeys, sizeof(lcb_store_cmd_t *));
	cmds = ecalloc(numkeys, sizeof(lcb_store_cmd_t));
	/* link the command pointers */
	for (ii = 0; ii < numkeys; ++ii) {
		commands[ii] = cmds + ii;
	}

	ctx = ecalloc(1, sizeof(php_couchbase_ctx));
	ctx->res = couchbase_res;
	ctx->rv	= return_value;
	array_init(ctx->rv);

	if (expire) {
		exp = pcbc_check_expiry(expire);
	}

	for (ii = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
			zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
			zend_hash_move_forward(Z_ARRVAL_P(akeys)), ++ii) {
		char *key = NULL;
		uint klen;
		size_t payload_len = 0;
		char *payload;
		unsigned int flags = 0;
		zval **ppzval;

		int key_type = zend_hash_get_current_key(Z_ARRVAL_P(akeys),
												 &key, NULL, 0);

		if (key_type != HASH_KEY_IS_STRING || (klen = strlen(key)) == 0) {
			int xx;
			for (xx = 0; xx < ii; ++xx) {
				efree((void *)cmds[xx].v.v0.bytes);
			}
			efree(commands);
			efree(cmds);
			efree(ctx);
			release_entry_array(entries,  xx);

			zend_throw_exception(cb_illegal_key_exception,
								 "Invalid key specified (not a string)",
								 0 TSRMLS_CC);
			return ;
		}

		if (zend_hash_get_current_data(Z_ARRVAL_P(akeys),
									   (void **)&ppzval) == FAILURE) {
			int xx;
			for (xx = 0; xx < ii; ++xx) {
				efree((void *)cmds[xx].v.v0.bytes);
			}
			efree(commands);
			efree(cmds);
			efree(ctx);
			release_entry_array(entries,  xx);

			zend_throw_exception(cb_exception,
								 "Failed to get data for key",
								 0 TSRMLS_CC);
			return ;
		}

		payload = php_couchbase_zval_to_payload(*ppzval, &payload_len, &flags,
												couchbase_res->serializer,
												couchbase_res->compressor
												TSRMLS_CC);

		if (payload == NULL) {
			/* Shouldn't we call an exception? */
			RETURN_FALSE;
		}

		if (couchbase_res->prefix_key_len) {
			char *new_key;
			klen = spprintf(&new_key, 0, "%s_%s", couchbase_res->prefix_key, key);
			key = new_key;
		}

		entries[ii].nkey = klen;
		entries[ii].key = emalloc(klen);
		memcpy(entries[ii].key, key, klen);
		cmds[ii].v.v0.operation = LCB_SET;
		cmds[ii].v.v0.key = entries[ii].key;
		cmds[ii].v.v0.nkey = klen;
		cmds[ii].v.v0.bytes = payload;
		cmds[ii].v.v0.nbytes = payload_len;
		cmds[ii].v.v0.flags = flags;
		cmds[ii].v.v0.exptime = exp;

		if (couchbase_res->prefix_key_len) {
			efree(key);
		}
	}

	retval = lcb_store(couchbase_res->handle, ctx, numkeys,
					   (const lcb_store_cmd_t * const *)commands);
	couchbase_res->seqno += numkeys;
	pcbc_start_loop(couchbase_res);

	/*
	 * Time to release the payloads...
	 */
	for (ii = 0; ii < numkeys; ++ii) {
		efree((void *)cmds[ii].v.v0.bytes);
	}
	efree(cmds);
	efree(commands);

	if (LCB_SUCCESS != retval) {
		efree(ctx);
		php_error_docref(NULL TSRMLS_CC, E_WARNING,
						 "Failed to schedule set request: %s",
						 lcb_strerror(couchbase_res->handle, retval));
		release_entry_array(entries,  numkeys);
		RETURN_FALSE;
	}

	/*
	 * The item was stored successfully. Did the user want to wait until
	 * it was persisted/replicated?
	 */
	if (persist_to != 0 || replicate_to != 0) {
		int ii = 0;
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(return_value));
				zend_hash_has_more_elements(Z_ARRVAL_P(return_value)) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(return_value)), ++ii) {
			zval **curr_cas;
			zend_hash_get_current_data(Z_ARRVAL_P(return_value),
									   (void **)&curr_cas);
			if (Z_STRLEN_PP(curr_cas)) {
				entries[ii].cas = strtoull(Z_STRVAL_PP(curr_cas), 0, 10);
			} else {
				/* @todo what to do here? */
				fprintf(stderr, "wtf!\n");
			}
		}

		retval = simple_observe(couchbase_res->handle, entries, numkeys,
								persist_to, replicate_to);
		couchbase_res->rc = retval;

		if (retval != LCB_SUCCESS) {
			if (retval == LCB_ETIMEDOUT) {
				zend_throw_exception(cb_timeout_exception,
									 "Timed out waiting for the objects to persist",
									 0 TSRMLS_CC);
			} else {
				char errmsg[256];
				snprintf(errmsg, sizeof(errmsg),
						 "An error occured while waiting for the objects to persist: %s",
						 lcb_strerror(couchbase_res->handle, retval));
				zend_throw_exception(cb_lcb_exception, errmsg, 0 TSRMLS_CC);
			}
		} else {
			int currsize = 4096;
			char *errmsg = malloc(currsize);
			int offset = sprintf(errmsg, "The following documents was mutated:");
			int errors = 0;

			for (ii = 0; ii < numkeys; ++ii) {
				if (entries[ii].mutated) {
					if ((offset + entries[ii].nkey + 3) > currsize) {
						char *p = realloc(errmsg, currsize * 2);
						if (p) {
							currsize *= 2;
							errmsg = p;
						}
					}

					if ((offset + entries[ii].nkey + 3) < currsize) {
						offset += sprintf(errmsg + offset, " \"");
						memcpy(errmsg + offset, entries[ii].key,
							   entries[ii].nkey);
						offset += entries[ii].nkey;
						offset += sprintf(errmsg + offset, "\"");
					}
					errors = 1;
				}
			}

			if (errors) {
				zend_throw_exception(cb_key_mutated_exception, errmsg,
									 0 TSRMLS_CC);
			}

			free(errmsg);
		}
	}

	release_entry_array(entries,  numkeys);
	efree(ctx);
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
