#include "internal.h"

/* {{{ static void php_couchbase_remove_callback(...) */
static void
php_couchbase_remove_callback(lcb_t instance,
                              const void *cookie,
                              lcb_error_t error,
                              const lcb_remove_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	php_ignore_value(instance);
	php_ignore_value(resp);

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
	}

	ctx->res->rc = error;
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAMETERS, int oo) /* {{{ */
{
	zval *akc = NULL, *adurability = NULL;
	char *key, *cas = NULL;
	long klen = 0, cas_len = 0;
	unsigned long long cas_v = 0;
	php_couchbase_res *couchbase_res;

	int argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
			"s|sa", &key, &klen, &cas, &cas_len, &adurability);
	{
		lcb_error_t retval;
		php_couchbase_ctx *ctx;

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;

		if (cas) {
			cas_v = strtoull(cas, 0, 10);
		}

		{
			lcb_remove_cmd_t cmd;
			lcb_remove_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.cas = cas_v;
			retval = lcb_remove(couchbase_res->handle, ctx,
			                    1, (const lcb_remove_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to schedule delete request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS == ctx->res->rc) {
			if (oo) {
				RETVAL_ZVAL(getThis(), 1, 0);
			} else {
				RETVAL_TRUE;
			}
		} else if (LCB_KEY_ENOENT == ctx->res->rc ||	/* skip missing key errors */
		           LCB_KEY_EEXISTS == ctx->res->rc) {	/* skip CAS mismatch */
			RETVAL_FALSE;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to remove a value from server: %s", lcb_strerror(couchbase_res->handle, ctx->res->rc));
			RETVAL_FALSE;
		}

		/* If we have a durability spec, after the commands have been issued (and callbacks returned), try to
		 * fulfill that spec by using polling observe internal:
		 */
		if (adurability != NULL) {
			array_init(akc);
			add_assoc_long(akc, key, cas_v);

			ctx->cas = akc;

			observe_polling_internal(ctx, adurability, 0);
		}

		efree(ctx);
	}
}
/* }}} */

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
