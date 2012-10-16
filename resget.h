#ifndef PHP_COUCHBASE_H
#error "Include php_couchbase.h first"
#endif

#ifndef PCBC_RESGET_H_
#define PCBC_RESGET_H_

/**
 * Flags for various calling styles with PHP entry points
 */
enum {
	/* no-op, but nice to have */
	PHP_COUCHBASE_ARG_F_FUNCTIONAL,

	/* this is a *multi variant */
	PHP_COUCHBASE_ARG_F_MULTI = 0x1,

	/* this is an oo variant */
	PHP_COUCHBASE_ARG_F_OO = 0x2
};

enum {
	/* all ok */
	PHP_COUCHBASE_RES_OK,

	/* caller should return immediately */
	PHP_COUCHBASE_RES_ERETURN,

	/* resource is invalid */
	PHP_COUCHBASE_RES_EINVAL,

	/* resource is not yet connected */
	PHP_COUCHBASE_RES_ENOTCONN,

	/* async operations in progress */
	PHP_COUCHBASE_RES_EBUSY,
};

PHP_COUCHBASE_LOCAL
void php_couchbase_get_resource(zval *r_or_this, int oo,
		int *ec,
		php_couchbase_res **pres,
		zval *return_value
		TSRMLS_DC);

PHP_COUCHBASE_LOCAL
int php_couchbase_res_ok(int ec TSRMLS_DC);

#define PHP_COUCHBASE_GET_RESOURCE(cbres, zvres, oo) \
{ \
	int __pcbc_ec = -1; \
	php_couchbase_get_resource((zvres), oo, &__pcbc_ec, &(cbres), return_value TSRMLS_CC); \
	if (__pcbc_ec == PHP_COUCHBASE_RES_ERETURN) { \
		return; \
	} else if (!php_couchbase_res_ok(__pcbc_ec TSRMLS_CC)) { \
		RETURN_FALSE; \
	} \
}



#define PHP_COUCHBASE_GET_RESOURCE_FUNCTIONAL(cbres, zvres) \
	PHP_COUCHBASE_GET_RESOURCE(cbres, zvres, 0)

#define PHP_COUCHBASE_GET_RESOURCE_OO(cbres) \
	PHP_COUCHBASE_GET_RESOURCE(cbres, getThis(), 1)

#endif /* PCBC_RESGET_H_ */
