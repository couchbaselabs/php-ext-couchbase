#include "internal.h"

void php_couchbase_get_resource(zval *r_or_this, int oo,
		int *ec,
		php_couchbase_res **pres,
		zval *return_value
		TSRMLS_DC)
{
	zval *zvres = NULL;
	*ec = PHP_COUCHBASE_RES_ERETURN;
	*pres = NULL;

	if (oo) {
		zvres = zend_read_property(couchbase_ce, r_or_this,
				ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
				1
				TSRMLS_CC);
		if (ZVAL_IS_NULL(zvres) || IS_RESOURCE != Z_TYPE_P(zvres)) {
			*ec = PHP_COUCHBASE_RES_EINVAL;
			return;
		}
	} else {
		zvres = r_or_this;
	}

	ZEND_FETCH_RESOURCE2(*pres, php_couchbase_res *, &zvres, -1,
			PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

	if (!(*pres)->is_connected) {
		*ec = PHP_COUCHBASE_RES_ENOTCONN;
		return;
	}

	if ((*pres)->async) {
		*ec = PHP_COUCHBASE_RES_EBUSY;
		return;
	}

	*ec = PHP_COUCHBASE_RES_OK;
}

int php_couchbase_res_ok(int ec TSRMLS_DC)
{
	switch(ec) {
	case PHP_COUCHBASE_RES_OK:
		return 1;

	case PHP_COUCHBASE_RES_ERETURN:
		return 0;

	case PHP_COUCHBASE_RES_ENOTCONN:
		php_error(E_WARNING, "There is no active connection to couchbase");
		return 0;

	case PHP_COUCHBASE_RES_EBUSY:
		php_error(E_WARNING, "There are some results that should be "
				"fetched before doing any sync operations");
		return 0;

	default:
		fprintf(stderr, "Unexpected code %d. abort\n", ec);
		abort();

		/* not reached */
		return -1;
	}
}
