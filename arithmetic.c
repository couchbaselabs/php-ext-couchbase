#include "internal.h"

/* {{{ static void php_couchbase_arithmetic_callback(...) */
static void
php_couchbase_arithmetic_callback(lcb_t instance,
                                  const void *cookie,
                                  lcb_error_t error,
                                  const lcb_arithmetic_resp_t *resp)
{
	php_couchbase_ctx *ctx = (php_couchbase_ctx *)cookie;
	uint64_t value;
	php_ignore_value(instance);

	if (--ctx->res->seqno == 0) {
		pcbc_stop_loop(ctx->res);
	}

	ctx->res->rc = error;
	if (LCB_SUCCESS != error) {
		return;
	}

	if (resp->version != 0) {
		ctx->res->rc = LCB_ERROR;
		return;
	}

	ZVAL_LONG(ctx->rv,	resp->v.v0.value);
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAMETERS, char op, int oo) /* {{{ */
{
	zval *res, *akc, *adurability = NULL;
	char *key;
	time_t exp = {0};
	long klen = 0, offset = 1, expire = 0;
	long create = 0, initial = 0;

	if (oo) {
		zval *self = getThis();
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|lllla", &key, &klen, &offset, &create, &expire, &initial, &adurability) == FAILURE) {
			return;
		}
		res = zend_read_property(couchbase_ce, self, ZEND_STRL(COUCHBASE_PROPERTY_HANDLE), 1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|lllla", &res, &key, &klen, &offset, &create, &expire, &initial, &adurability) == FAILURE) {
			return;
		}
	}
	{
		lcb_error_t retval;
		php_couchbase_res *couchbase_res;
		php_couchbase_ctx *ctx;
		long delta = (op == '+') ? offset : -offset;

		ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1, PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);
		if (!couchbase_res->is_connected) {
			php_error(E_WARNING, "There is no active connection to couchbase.");
			RETURN_FALSE;
		}
		if (couchbase_res->async) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "there are some results should be fetched before do any sync request");
			RETURN_FALSE;
		}

		if (expire) {
			exp = pcbc_check_expiry(expire);
		}

		ctx = ecalloc(1, sizeof(php_couchbase_ctx));
		ctx->res = couchbase_res;
		ctx->rv = return_value;

		{
			lcb_arithmetic_cmd_t cmd;
			lcb_arithmetic_cmd_t *commands[] = { &cmd };
			memset(&cmd, 0, sizeof(cmd));
			cmd.v.v0.key = key;
			cmd.v.v0.nkey = klen;
			cmd.v.v0.create = create;
			cmd.v.v0.delta = delta;
			cmd.v.v0.initial = initial;
			cmd.v.v0.exptime = exp;

			retval = lcb_arithmetic(couchbase_res->handle, ctx, 1,
			                        (const lcb_arithmetic_cmd_t * const *)commands);
		}
		if (LCB_SUCCESS != retval) {
			efree(ctx);
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
			                 "Failed to schedule rithmetic request: %s", lcb_strerror(couchbase_res->handle, retval));
			RETURN_FALSE;
		}

		couchbase_res->seqno += 1;
		pcbc_start_loop(couchbase_res);
		if (LCB_SUCCESS != ctx->res->rc) {
			// Just return false and don't print a warning when no key is present and create is false
			if (!(LCB_KEY_ENOENT == ctx->res->rc && create == 0)) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING,
				                 "Failed to %s value in server: %s", (op == '+') ? "increment" : "decrement", lcb_strerror(couchbase_res->handle, ctx->res->rc));
			}
			efree(ctx);
			RETURN_FALSE;
		}

		/* If we have a durability spec, after the commands have been issued (and callbacks returned), try to
		 * fulfill that spec by using polling observe internal:
		 */
		if (adurability != NULL) {
			array_init(akc);
			add_assoc_long(akc, key, Z_LVAL_P(ctx->cas));

			zval_dtor(ctx->cas);
			ctx->cas = akc;

			observe_polling_internal(ctx, adurability, 0);
		}
		efree(ctx);
	}
}
/* }}} */

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_arithmetic_init(lcb_t handle)
{
	php_ignore_value(
			lcb_set_arithmetic_callback(handle, php_couchbase_arithmetic_callback));
}
