/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.                                       |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");      |
  | you may not use this file except in compliance with the License.     |
  | You may obtain a copy of the License at                              |
  |     http://www.apache.org/licenses/LICENSE-2.0                       |
  | Unless required by applicable law or agreed to in writing, software  |
  | distributed under the License is distributed on an "AS IS" BASIS,    |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or      |
  | implied. See the License for the specific language governing         |
  | permissions and limitations under the License.                       |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

void php_couchbase_get_resource(INTERNAL_FUNCTION_PARAMETERS,
								zval *zvres,
								int argflags,
								int *ec,
								php_couchbase_res **pres)
{
	*ec = PHP_COUCHBASE_RES_ERETURN;
	*pres = NULL;

	if (argflags & PHP_COUCHBASE_ARG_F_OO) {
		zvres = zend_read_property(couchbase_ce, getThis(),
								   ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
								   1
								   TSRMLS_CC);
		if (ZVAL_IS_NULL(zvres) || IS_RESOURCE != Z_TYPE_P(zvres)) {
			*ec = PHP_COUCHBASE_RES_EINVAL;
			return;
		}
	}

	ZEND_FETCH_RESOURCE2(*pres, php_couchbase_res *, &zvres, -1,
						 PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

	if ((argflags & PHP_COUCHBASE_ARG_F_NOCONN) == 0
			&& (*pres)->is_connected == 0) {
		*ec = PHP_COUCHBASE_RES_ENOTCONN;
		return;
	}

	if ((argflags & PHP_COUCHBASE_ARG_F_ASYNC) == 0  && (*pres)->async) {
		*ec = PHP_COUCHBASE_RES_EBUSY;
		return;
	}

	*ec = PHP_COUCHBASE_RES_OK;
}

int php_couchbase_res_ok(int ec TSRMLS_DC)
{
	switch (ec) {
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

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
