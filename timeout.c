/*
  +----------------------------------------------------------------------+
  | PHP Version 5														 |
  +----------------------------------------------------------------------+
  | Copyright 2012 Couchbase, Inc.										 |
  +----------------------------------------------------------------------+
  | Licensed under the Apache License, Version 2.0 (the "License");		 |
  | you may not use this file except in compliance with the License.	 |
  | You may obtain a copy of the License at								 |
  |		http://www.apache.org/licenses/LICENSE-2.0						 |
  | Unless required by applicable law or agreed to in writing, software	 |
  | distributed under the License is distributed on an "AS IS" BASIS,	 |
  | WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or		 |
  | implied. See the License for the specific language governing		 |
  | permissions and limitations under the License.						 |
  +----------------------------------------------------------------------+
  | Author: Mark Nunberg	   <mnunberg@haskalah.org>					 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

PHP_COUCHBASE_LOCAL
void php_couchbase_get_timeout_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	zval *res;
	php_couchbase_res *couchbase_res;


	if (oo) {
		zval *self = getThis();
		res = zend_read_property(couchbase_ce, self,
		                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
		                         1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		                          "r", &res) == FAILURE) {
			return;
		}
	}

	ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1,
	                     PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

	RETURN_LONG(lcb_get_timeout(couchbase_res->handle));
}

PHP_COUCHBASE_LOCAL
void php_couchbase_set_timeout_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	zval *res;
	php_couchbase_res *couchbase_res;
	long tmo;


	if (oo) {
		zval *self = getThis();
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		                          "l", &tmo) == FAILURE) {
			return;
		}
		res = zend_read_property(couchbase_ce, self,
		                         ZEND_STRL(COUCHBASE_PROPERTY_HANDLE),
		                         1 TSRMLS_CC);
		if (ZVAL_IS_NULL(res) || IS_RESOURCE != Z_TYPE_P(res)) {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "unintilized couchbase");
			RETURN_FALSE;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,
		                          "rl", &res, &tmo) == FAILURE) {
			return;
		}
	}

	ZEND_FETCH_RESOURCE2(couchbase_res, php_couchbase_res *, &res, -1,
	                     PHP_COUCHBASE_RESOURCE, le_couchbase, le_pcouchbase);

	lcb_set_timeout(couchbase_res->handle, (lcb_uint32_t)tmo);
	RETURN_TRUE;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
