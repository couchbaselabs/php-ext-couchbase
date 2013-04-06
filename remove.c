#include "internal.h"

struct remove_cookie {
	lcb_error_t error;
	uint64_t cas;
};

static void php_couchbase_remove_callback(lcb_t instance,
										  const void *cookie,
										  lcb_error_t error,
										  const lcb_remove_resp_t *resp)
{
	struct remove_cookie *rmc = (struct remove_cookie *)cookie;
	rmc->error = error;
	rmc->cas = resp->v.v0.cas;
	if (resp->version != 0) {
		rmc->error = LCB_ERROR;
	}
}

static lcb_error_t do_remove(lcb_t instance, const void *key, uint16_t klen,
							 lcb_cas_t *cas)
{
	lcb_remove_cmd_t cmd;
	const lcb_remove_cmd_t *const commands[] = { &cmd };
	struct remove_cookie rmc;
	lcb_error_t retval;

	memset(&cmd, 0, sizeof(cmd));
	memset(&rmc, 0, sizeof(rmc));
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = klen;
	cmd.v.v0.cas = *cas;

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_remove(instance, &rmc, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS && rmc.error == LCB_SUCCESS) {
		*cas = rmc.cas;
	}

	return (retval == LCB_SUCCESS) ? rmc.error : retval;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	char *key;
	char *cas = NULL;
	long klen = 0;
	long cas_len = 0;
	long replicate_to = 0;
	long persist_to = 0;
	lcb_cas_t cas_v = 0;
	php_couchbase_res *couchbase_res;
	lcb_error_t retval;
	php_couchbase_ctx *ctx;
	char errmsg[256];

	int arg = (oo) ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res,  arg,
							 "s|sll", &key, &klen, &cas, &cas_len,
							 &persist_to, &replicate_to);

	if (klen == 0) {
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_illegal_key_exception,
							   "No key specified: Empty key");
		return ;
	}

	if (validate_simple_observe_clause(couchbase_res->handle,
									   persist_to,
									   replicate_to TSRMLS_CC) == -1) {
		/* Exception already thrown */
		return;
	}

	if (cas_len > 0) {
		char *e;
		cas_v = (lcb_cas_t)strtoull(cas, &e, 10);
		if (*e != '\0') {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_illegal_key_exception,
								   "Invalid CAS specified");
			return;
		}
	}

	retval = do_remove(couchbase_res->handle, key, klen, &cas_v);
	couchbase_res->rc = retval;

	switch (retval) {
	case LCB_SUCCESS:
		Z_TYPE_P(return_value) = IS_STRING;
		Z_STRLEN_P(return_value) = spprintf(&(Z_STRVAL_P(return_value)), 0,
											"%llu", cas_v);
		break;
	case LCB_KEY_ENOENT:
		RETURN_FALSE;
		/* NOTREACHED */
	case LCB_KEY_EEXISTS:
		if (oo) {
			couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
								   cb_key_mutated_exception,
								   "Failed to remove the value from the server: %s",
								   lcb_strerror(couchbase_res->handle, retval));
			return ;
		} else {
			RETURN_FALSE;
		}
	default:
		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to remove the value from the server: %s",
							   lcb_strerror(couchbase_res->handle, retval));
		return ;
	}

	if (retval == LCB_SUCCESS && (persist_to > 0 || replicate_to > 0)) {
		/*
		 * If we have a durability spec, after the commands have been
		 * issued (and callbacks returned), try to fulfill that spec by
		 * using polling observe internal (please note that this is
		 * only possible from OO)
		 */
		struct observe_entry entry;
		memset(&entry, 0, sizeof(entry));
		entry.key = key;
		entry.nkey = klen;
		entry.cas = cas_v;

		retval = simple_observe(couchbase_res->handle, &entry, 1,
								persist_to, replicate_to);
		couchbase_res->rc = retval;

		if (retval != LCB_SUCCESS) {
			if (retval == LCB_ETIMEDOUT) {
				zend_throw_exception(cb_timeout_exception,
									 "Timed out waiting for the objects to persist",
									 0 TSRMLS_CC);
			} else {
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

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_remove_init(lcb_t handle)
{
	lcb_set_remove_callback(handle, php_couchbase_remove_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
