#include "internal.h"

static void php_couchbase_get_callback(lcb_t instance,
									   const void *cookie,
									   lcb_error_t error,
									   const lcb_get_resp_t *resp)
{
	zval *retval;
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	const void *key;
	size_t nkey;
	const void *bytes;
	size_t nbytes;
	uint32_t flags;
	uint64_t cas;


	TSRMLS_FETCH();

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}
	key = resp->v.v0.key;
	nkey = resp->v.v0.nkey;
	bytes = resp->v.v0.bytes;
	nbytes = resp->v.v0.nbytes;
	flags = resp->v.v0.flags;
	cas = resp->v.v0.cas;

	ctx->res->rc = error;
	if (LCB_SUCCESS != error && LCB_KEY_ENOENT != error) {
		pcbc_stop_loop(ctx->res);
		return;
	}

	if (ctx->res->async) { /* get_delayed */
		zval *k, *v;
		MAKE_STD_ZVAL(v);
		if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes, flags,
											 ctx->res->serializer, ctx->res->ignoreflags
											 TSRMLS_CC)) {
			ctx->res->rc = LCB_ERROR;
			efree(v);
			return;
		}

		if (ctx->res->prefix_key_len && nkey) {
			if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
				nkey -= (ctx->res->prefix_key_len + 1); /* '_' */
				key = estrndup(((const char *)key) + ctx->res->prefix_key_len + 1, nkey);
			}
		}

		MAKE_STD_ZVAL(retval);
		array_init(retval);
		zend_hash_next_index_insert(Z_ARRVAL_P(ctx->rv), (void **)&retval,
									sizeof(zval *), NULL);

		MAKE_STD_ZVAL(k);
		ZVAL_STRINGL(k, (char *)key, nkey, 1);

		zend_hash_add(Z_ARRVAL_P(retval), "key", sizeof("key"),
					  (void **)&k, sizeof(zval *), NULL);
		zend_hash_add(Z_ARRVAL_P(retval), "value", sizeof("value"),
					  (void **)&v, sizeof(zval *), NULL);

		if (ctx->flags) {
			zval *c;
			MAKE_STD_ZVAL(c);
			Z_TYPE_P(c) = IS_STRING;
			Z_STRLEN_P(c) = spprintf(&(Z_STRVAL_P(c)), 0, "%llu", cas);
			zend_hash_add(Z_ARRVAL_P(retval), "cas", sizeof("cas"),
						  (void **)&c, sizeof(zval *), NULL);
		}

		if (ctx->res->prefix_key_len && nkey) {
			efree((void *)key);
		}
	} else {
		if (LCB_KEY_ENOENT == error) {
			return;
		}

		if (IS_ARRAY == Z_TYPE_P(ctx->rv)) { /* multi get */
			zval *v;
			char *key_string = NULL;
			MAKE_STD_ZVAL(v);
			if (!php_couchbase_zval_from_payload(v, (char *)bytes, nbytes,
												 flags, ctx->res->serializer,
												 ctx->res->ignoreflags TSRMLS_CC)) {
				ctx->res->rc = LCB_ERROR;
				efree(v);
				return;
			}

			if (ctx->res->prefix_key_len && nkey) {
				if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
					nkey -= (ctx->res->prefix_key_len + 1);
					key_string = estrndup(((const char *)key) +
										  ctx->res->prefix_key_len + 1, nkey);
				}
			} else {
				key_string = emalloc(nkey + 1);
				memcpy(key_string, key, nkey);
				key_string[nkey] = '\0';
			}
			zend_hash_update((Z_ARRVAL_P(ctx->rv)), (char *)key_string, nkey + 1,
							 (void **)&v, sizeof(zval *), NULL);

			if (ctx->cas) {
				zval *c;
				MAKE_STD_ZVAL(c);
				Z_TYPE_P(c) = IS_STRING;
				Z_STRLEN_P(c) = spprintf(&(Z_STRVAL_P(c)), 0, "%llu", cas);
				zend_hash_add(Z_ARRVAL_P(ctx->cas), (char *)key_string, nkey + 1,
							  (void **)&c, sizeof(zval *), NULL);
			}
			efree(key_string);
		} else {
			if (ctx->res->prefix_key_len && nkey) {
				if (!strncmp(key, ctx->res->prefix_key, ctx->res->prefix_key_len)) {
					nkey -= (ctx->res->prefix_key_len + 1);
					key = estrndup(((const char *)key) + ctx->res->prefix_key_len + 1,
								   nkey);
				}
			}
			if (!php_couchbase_zval_from_payload(ctx->rv, (char *)bytes, nbytes,
												 flags, ctx->res->serializer,
												 ctx->res->ignoreflags TSRMLS_CC)) {
				if (ctx->res->prefix_key_len && nkey) {
					efree((void *)key);
				}
				ctx->res->rc = LCB_ERROR;
				return;
			}
			if (ctx->res->prefix_key_len && nkey) {
				efree((void *)key);
			}
			if (ctx->cas) {
				Z_TYPE_P(ctx->cas) = IS_STRING;
				Z_STRLEN_P(ctx->cas) = spprintf(&(Z_STRVAL_P(ctx->cas)), 0, "%llu", cas);
			}
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_get_impl(INTERNAL_FUNCTION_PARAMETERS,
							int multi, int oo, int lock, int touch, int replica)
{
	char *key, **keys;
	long *klens, klen = 0;
	int	 nkey = 0;
	long flag = 0;
	lcb_time_t exp = {0};
	long expiry = 0;
	zval *res, *cas_token = NULL;
	int argflags;
	lcb_error_t retval;
	php_couchbase_res *couchbase_res;
	php_couchbase_ctx *ctx;
	zend_fcall_info fci = {0};
	zend_fcall_info_cache fci_cache;

	argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	if (multi) {
		zval *akeys;
		zval **ppzval;
		zend_bool preserve_order;
		int i;

		if (lock) {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "az|ll",
											 &akeys, &cas_token,
											 &flag, &expiry);
		} else if (touch) {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "al|z",
											 &akeys, &expiry, &cas_token);
		} else {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "a|zl",
											 &akeys, &cas_token, &flag);
		}

		nkey = zend_hash_num_elements(Z_ARRVAL_P(akeys));
		keys = ecalloc(nkey, sizeof(char *));
		klens = ecalloc(nkey, sizeof(long));
		preserve_order = (flag & COUCHBASE_GET_PRESERVE_ORDER);

		array_init(return_value);

		for (i = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
				zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(akeys)), i++) {
			if (zend_hash_get_current_data(Z_ARRVAL_P(akeys), (void **)&ppzval) == FAILURE) {
				nkey--;
				continue;
			}

			if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
				convert_to_string_ex(ppzval);
			}

			if (!Z_STRLEN_PP(ppzval)) {
				nkey--;
				continue;
			}

			if (couchbase_res->prefix_key_len) {
				klens[i] = spprintf(&(keys[i]), 0, "%s_%s", couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
			} else {
				keys[i] = Z_STRVAL_PP(ppzval);
				klens[i] = Z_STRLEN_PP(ppzval);
			}

			if (preserve_order) {
				add_assoc_null_ex(return_value, keys[i], klens[i] + 1);
			}
		}

		if (!nkey) {
			efree(keys);
			efree(klens);
			return;
		}

		if (cas_token && IS_ARRAY != Z_TYPE_P(cas_token)) {
			zval_dtor(cas_token);
			array_init(cas_token);
		}
	} else {
		if (lock) {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "sz|l", &key, &klen,
											 &cas_token, &expiry);
		} else if (touch) {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "sl|z", &key, &klen, &expiry,
											 &cas_token);
		} else {
			PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
											 "s|f!z", &key, &klen,
											 &fci, &fci_cache,
											 &cas_token);
		}

		if (!klen) {
			return;
		}

		nkey = 1;
		if (couchbase_res->prefix_key_len) {
			klen = spprintf(&key, 0, "%s_%s", couchbase_res->prefix_key, key);
		}
		keys = &key;
		klens = &klen;

		if (cas_token) {
			zval_dtor(cas_token);
			ZVAL_NULL(cas_token);
		}
	}
	if (replica) {
		lcb_get_replica_cmd_t **commands = ecalloc(nkey, sizeof(lcb_get_replica_cmd_t *));
		int ii;

		for (ii = 0; ii < nkey; ++ii) {
			lcb_get_replica_cmd_t *cmd = ecalloc(1, sizeof(lcb_get_replica_cmd_t));
			commands[ii] = cmd;
			cmd->v.v0.key = keys[ii];
			cmd->v.v0.nkey = klens[ii];
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv	 = return_value;
		ctx->cas = cas_token;

		retval = lcb_get_replica(couchbase_res->handle, ctx,
								 nkey, (const lcb_get_replica_cmd_t * const *)commands);
		for (ii = 0; ii < nkey; ++ii) {
			efree(commands[ii]);
		}
		efree(commands);
	} else {
		lcb_get_cmd_t **commands = ecalloc(nkey, sizeof(lcb_get_cmd_t *));
		int ii;

		if (expiry) {
			exp = pcbc_check_expiry(expiry);
		}

		for (ii = 0; ii < nkey; ++ii) {
			lcb_get_cmd_t *cmd = ecalloc(1, sizeof(lcb_get_cmd_t));
			commands[ii] = cmd;
			cmd->v.v0.key = keys[ii];
			cmd->v.v0.nkey = klens[ii];
			cmd->v.v0.lock = (int)lock;
			cmd->v.v0.exptime = exp; /* NB: this assumes sizeof(lcb_time_t) == sizeof(long) */
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv	 = return_value;
		ctx->cas = cas_token;

		retval = lcb_get(couchbase_res->handle, ctx,
						 nkey, (const lcb_get_cmd_t * const *)commands);
		for (ii = 0; ii < nkey; ++ii) {
			efree(commands[ii]);
		}
		efree(commands);
	}

	if (LCB_SUCCESS != retval) {
		if (couchbase_res->prefix_key_len) {
			int i;
			for (i = 0; i < nkey; i++) {
				efree(keys[i]);
			}
		}
		if (multi) {
			efree(keys);
			efree(klens);
			zval_dtor(return_value);
		}
		efree(ctx);

		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to schedule get request: %s",
							   lcb_strerror(couchbase_res->handle, retval));
		return;
	}

	couchbase_res->seqno += nkey;
	pcbc_start_loop(couchbase_res);
	if (ctx->res->rc != LCB_SUCCESS) {
		if (!multi) {
			RETVAL_FALSE;
		}
		if (ctx->res->rc == LCB_KEY_ENOENT) {
			if (fci.size) {
				zval *result;
				zval *zkey;
				zval *retval_ptr = NULL;
				zval **params[3];
				int cbret;

				MAKE_STD_ZVAL(result);
				MAKE_STD_ZVAL(zkey);
				ZVAL_NULL(result);
				ZVAL_STRINGL(zkey, key, klen, 1);

				params[0] = &res;
				params[1] = &zkey;
				params[2] = &result;
				fci.retval_ptr_ptr = &retval_ptr;
				fci.param_count = 3;
				fci.params = params;

				cbret = zend_call_function(&fci, &fci_cache TSRMLS_CC);

				if (cbret == SUCCESS && fci.retval_ptr_ptr && *fci.retval_ptr_ptr) {
					if (Z_TYPE_P(retval_ptr) == IS_BOOL && Z_BVAL_P(retval_ptr)) {
						zval_ptr_dtor(fci.retval_ptr_ptr);
						zval_ptr_dtor(&zkey);
						efree(ctx);
						if (multi) {
							zval_dtor(return_value);
						}
						RETURN_ZVAL(result, 1, 1);
					}
					zval_ptr_dtor(fci.retval_ptr_ptr);
				}
				zval_ptr_dtor(&zkey);
				zval_ptr_dtor(&result);
			}
		} else {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_lcb_exception,
								   "Failed to get a value from server: %s",
								   lcb_strerror(couchbase_res->handle,
												ctx->res->rc));
		}
	}
	efree(ctx);
	if (couchbase_res->prefix_key_len) {
		int i;
		for (i = 0; i < nkey; i++) {
			efree(keys[i]);
		}
	}
	if (multi) {
		efree(keys);
		efree(klens);
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_get_delayed_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	zval *res, *akeys;
	long with_cas = 0;
	lcb_time_t exp = {0};
	long expiry = 0;
	zend_bool lock = 0;
	int argflags = PHP_COUCHBASE_ARG_F_ASYNC;
	php_couchbase_res *couchbase_res;
	zend_fcall_info fci = {0};
	zend_fcall_info_cache fci_cache = {0};
	zval **ppzval;
	lcb_error_t retval;
	php_couchbase_ctx *ctx;
	char **keys;
	long nkey, *klens, i;
	lcb_get_cmd_t **commands;
	int ii;

	if (oo) {
		argflags |= PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags |= PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	PHP_COUCHBASE_GET_PARAMS_WITH_ZV(res, couchbase_res, argflags,
									 "a|lf!lb",
									 &akeys, &with_cas, &fci, &fci_cache,
									 &expiry, &lock);

	nkey = zend_hash_num_elements(Z_ARRVAL_P(akeys));
	keys = ecalloc(nkey, sizeof(char *));
	klens = ecalloc(nkey, sizeof(long));

	for (i = 0, zend_hash_internal_pointer_reset(Z_ARRVAL_P(akeys));
			zend_hash_has_more_elements(Z_ARRVAL_P(akeys)) == SUCCESS;
			zend_hash_move_forward(Z_ARRVAL_P(akeys)), i++) {
		if (zend_hash_get_current_data(Z_ARRVAL_P(akeys),
									   (void **)&ppzval) == FAILURE) {
			nkey--;
			continue;
		}

		if (IS_ARRAY != Z_TYPE_PP(ppzval)) {
			convert_to_string_ex(ppzval);
		}

		if (!Z_STRLEN_PP(ppzval)) {
			nkey--;
			continue;
		}

		if (couchbase_res->prefix_key_len) {
			klens[i] = spprintf(&(keys[i]), 0, "%s_%s",
								couchbase_res->prefix_key, Z_STRVAL_PP(ppzval));
		} else {
			keys[i] = Z_STRVAL_PP(ppzval);
			klens[i] = Z_STRLEN_PP(ppzval);
		}
	}

	if (!nkey) {
		efree(keys);
		efree(klens);
		return;
	}

	couchbase_res->seqno += nkey;
	ctx = ecalloc(1, sizeof(php_couchbase_ctx));
	ctx->res = couchbase_res;
	ctx->flags = with_cas;

	commands = ecalloc(nkey, sizeof(lcb_get_cmd_t *));
	if (expiry) {
		exp = pcbc_check_expiry(expiry);
	}

	for (ii = 0; ii < nkey; ++ii) {
		lcb_get_cmd_t *cmd = ecalloc(1, sizeof(lcb_get_cmd_t));
		commands[ii] = cmd;
		cmd->v.v0.key = keys[ii];
		cmd->v.v0.nkey = klens[ii];
		cmd->v.v0.lock = (int)lock;
		cmd->v.v0.exptime = exp; /* NB: this assumes that sizeof(lcb_time_t) == sizeof(long) */
	}

	retval = lcb_get(couchbase_res->handle, ctx,
					 nkey, (const lcb_get_cmd_t * const *)commands);
	for (ii = 0; ii < nkey; ++ii) {
		efree(commands[ii]);
	}
	efree(commands);

	if (LCB_SUCCESS != retval) {
		if (couchbase_res->prefix_key_len) {
			for (ii = 0; ii < nkey; ii++) {
				efree(keys[ii]);
			}
		}
		efree(keys);
		efree(klens);
		efree(ctx);
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to schedule delayed get request: %s",
							   lcb_strerror(couchbase_res->handle, retval));
		return;
	}
	couchbase_res->async = 1;
	couchbase_res->async_ctx = ctx;
	if (couchbase_res->prefix_key_len) {
		for (ii = 0; ii < nkey; ii++) {
			efree(keys[ii]);
		}
	}
	efree(keys);
	efree(klens);
	if (fci.size) {
		zval *result, **ppzval, *retval_ptr = NULL;
		zval **params[2];

		MAKE_STD_ZVAL(result);
		array_init(result);
		ctx->rv = result;
		pcbc_start_loop(couchbase_res);
		couchbase_res->async = 0;
		for (zend_hash_internal_pointer_reset(Z_ARRVAL_P(result));
				zend_hash_has_more_elements(Z_ARRVAL_P(result)) == SUCCESS;
				zend_hash_move_forward(Z_ARRVAL_P(result))) {
			if (zend_hash_get_current_data(Z_ARRVAL_P(result),
										   (void **)&ppzval) == FAILURE) {
				continue;
			}

			params[0] = &res;
			params[1] = ppzval;
			fci.retval_ptr_ptr = &retval_ptr;
			fci.param_count = 2;
			fci.params = params;
			zend_call_function(&fci, &fci_cache TSRMLS_CC);
			if (retval_ptr != NULL) {
				zval_ptr_dtor(&retval_ptr);
			}
		}
		zval_ptr_dtor(&result);
		efree(ctx);
	}
	RETURN_TRUE;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo)
{
	php_couchbase_res *couchbase_res;
	int argflags;

	if (oo) {
		argflags = PHP_COUCHBASE_ARG_F_OO;
	} else {
		argflags = PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	}

	argflags |= PHP_COUCHBASE_ARG_F_ASYNC;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "");

	{
		php_couchbase_ctx *ctx;

		if (!couchbase_res->async) {
			RETURN_FALSE;
		}

		ctx = couchbase_res->async_ctx;

		if (couchbase_res->async == 2) {
fetch_one: {
				char *key;
				uint key_len;
				ulong index = 0;
				zval **ppzval;
				zval *stash = (zval *)ctx->extended_value;
				if (zend_hash_num_elements(Z_ARRVAL_P(stash)) == 0) {
					couchbase_res->async = 0;
					zval_ptr_dtor(&stash);
					efree(ctx);
					couchbase_res->async_ctx = NULL;
					RETURN_NULL();
				}
				zend_hash_internal_pointer_reset(Z_ARRVAL_P(stash));
				zend_hash_get_current_data(Z_ARRVAL_P(stash), (void **)&ppzval);
				RETVAL_ZVAL(*ppzval, 1, 0);
				zend_hash_get_current_key_ex(Z_ARRVAL_P(stash), &key, &key_len, &index, 0, NULL);
				zend_hash_index_del(Z_ARRVAL_P(stash), index);
				return;
			}
		}

		array_init(return_value);
		ctx->rv = return_value;
		pcbc_start_loop(couchbase_res);
		if (!multi) {
			zval *stash;
			MAKE_STD_ZVAL(stash);
			ZVAL_ZVAL(stash, return_value, 1, 0);
			ctx->extended_value = (void *)stash;
			zval_dtor(return_value);
			couchbase_res->async = 2;
			goto fetch_one;
		} else {
			efree(ctx);
			couchbase_res->async = 0;
			couchbase_res->async_ctx = NULL;
		}
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_get_init(lcb_t handle)
{
	lcb_set_get_callback(handle, php_couchbase_get_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
