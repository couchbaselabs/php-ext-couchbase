#include "internal.h"

struct arithmetic_cookie {
	lcb_error_t error;
	uint64_t value;
};

static void php_couchbase_arithmetic_callback(lcb_t instance,
											  const void *cookie,
											  lcb_error_t error,
											  const lcb_arithmetic_resp_t *resp)
{
	struct arithmetic_cookie *c = (struct arithmetic_cookie *)cookie;
	c->error = error;
	if (resp->version != 0) {
		c->error = LCB_ERROR;
	} else {
		c->value = resp->v.v0.value;
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAMETERS, char op, int oo)
{
	char *key;
	time_t exp;
	long klen = 0;
	long offset = 1;
	long expire = 0;
	long create = 0;
	long initial = 0;
	php_couchbase_res *couchbase_res;
	lcb_arithmetic_cmd_t cmd;
	const lcb_arithmetic_cmd_t *const commands[] = { &cmd };
	lcb_error_t retval;
	struct arithmetic_cookie cookie;
	int argflags;
	long delta;
	lcb_t instance;

	argflags = oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL;
	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags,
							 "s|llll", &key, &klen, &offset, &create,
							 &expire, &initial);
	instance = couchbase_res->handle;

	memset(&cookie, 0, sizeof(cookie));
	delta = (op == '+') ? offset : -offset;

	if (expire) {
		exp = pcbc_check_expiry(expire);
	} else {
		exp = 0;
	}

	memset(&cmd, 0, sizeof(cmd));
	cmd.v.v0.key = key;
	cmd.v.v0.nkey = klen;
	cmd.v.v0.create = create;
	cmd.v.v0.delta = delta;
	cmd.v.v0.initial = initial;
	cmd.v.v0.exptime = exp;

	lcb_behavior_set_syncmode(instance, LCB_SYNCHRONOUS);
	retval = lcb_arithmetic(instance, &cookie, 1, commands);
	lcb_behavior_set_syncmode(instance, LCB_ASYNCHRONOUS);

	if (retval == LCB_SUCCESS) {
		retval = cookie.error ;
	}
	couchbase_res->rc = retval;

	if (retval == LCB_SUCCESS) {
		ZVAL_LONG(return_value, cookie.value);
	} else {
		if (retval == LCB_KEY_ENOENT && create == 0) {
			/* The user told us to not create the key... */
			RETURN_FALSE;
		}

		couchbase_report_error(INTERNAL_FUNCTION_PARAM_PASSTHRU, oo,
							   cb_lcb_exception,
							   "Failed to %s value in the server: %s",
							   (op == '+') ? "increment" : "decrement",
							   lcb_strerror(instance, retval));
	}
}

PHP_COUCHBASE_LOCAL
void php_couchbase_callbacks_arithmetic_init(lcb_t handle)
{
	(void)lcb_set_arithmetic_callback(handle, php_couchbase_arithmetic_callback);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
