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
  | Author: Trond Norbye	   <trond.norbye@gmail.com>					 |
  +----------------------------------------------------------------------+
*/
#include "internal.h"

PHP_COUCHBASE_LOCAL
void php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	char errmsg[256];
	long option;
	zval *value;
	php_couchbase_res *couchbase_res;
	int argflags =
		(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
		PHP_COUCHBASE_ARG_F_NOCONN | PHP_COUCHBASE_ARG_F_ASYNC;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "lz", &option, &value);

	switch (option) {
	case COUCHBASE_OPT_SERIALIZER:
		convert_to_long_ex(&value);
		switch (Z_LVAL_P(value)) {
		case COUCHBASE_SERIALIZER_PHP:
		case COUCHBASE_SERIALIZER_JSON:
		case COUCHBASE_SERIALIZER_JSON_ARRAY:
#ifdef HAVE_JSON_API
			couchbase_res->serializer = Z_LVAL_P(value);
			if (oo) {
				RETURN_ZVAL(getThis(), 1, 0);
			}
			RETURN_TRUE;
#else
			snprintf(errmsg, sizeof(errmsg), "%s",
					 "json serializer is not supported");

			if (oo) {
				zend_throw_exception(cb_illegal_value_exception,
									 errmsg, 0 TSRMLS_CC);
				return ;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
				RETURN_FALSE;
			}
#endif
			break;
		default:
			snprintf(errmsg, sizeof(errmsg),
					 "unsupported serializer: %ld", Z_LVAL_P(value));
			if (oo) {
				zend_throw_exception(cb_illegal_value_exception,
									 errmsg, 0 TSRMLS_CC);
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
				RETURN_FALSE;
			}
		}
		break;

	case COUCHBASE_OPT_PREFIX_KEY:
		convert_to_string_ex(&value);
		if (couchbase_res->prefix_key) {
			efree(couchbase_res->prefix_key);
		}
		couchbase_res->prefix_key = estrndup(Z_STRVAL_P(value),
											 Z_STRLEN_P(value));
		couchbase_res->prefix_key_len = Z_STRLEN_P(value);
		break;

	case COUCHBASE_OPT_COMPRESSION:
		convert_to_long_ex(&value);
		switch (Z_LVAL_P(value)) {
		case COUCHBASE_COMPRESSION_NONE:
		case COUCHBASE_COMPRESSION_FASTLZ:
		case COUCHBASE_COMPRESSION_ZLIB:
			couchbase_res->compressor = Z_LVAL_P(value);
			if (oo) {
				RETURN_ZVAL(getThis(), 1, 0);
			}
			RETURN_TRUE;
			break;
		default:
			snprintf(errmsg, sizeof(errmsg), "unsupported compressor: %ld",
					 Z_LVAL_P(value));
			if (oo) {
				zend_throw_exception(cb_illegal_value_exception,
									 errmsg, 0 TSRMLS_CC);
				return;
			} else {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
				RETURN_FALSE;
			}
		}
		break;

	case COUCHBASE_OPT_IGNOREFLAGS:
		convert_to_long_ex(&value);
		couchbase_res->ignoreflags = Z_LVAL_P(value);
		break;

	case COUCHBASE_OPT_VOPTS_PASSTHROUGH:
		convert_to_long_ex(&value);
		couchbase_res->viewopts_passthrough = Z_LVAL_P(value);
		break;

	default:
		snprintf(errmsg, sizeof(errmsg), "unknown option type: %ld", option);
		if (oo) {
			zend_throw_exception(cb_illegal_option_exception,
								 errmsg, 0 TSRMLS_CC);

			return;
		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", errmsg);
			RETURN_FALSE;
		}
	}
	RETURN_TRUE;
}

PHP_COUCHBASE_LOCAL
void php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo)
{
	long option;
	php_couchbase_res *couchbase_res;
	int argflags =
		(oo ? PHP_COUCHBASE_ARG_F_OO : PHP_COUCHBASE_ARG_F_FUNCTIONAL) |
		PHP_COUCHBASE_ARG_F_ONLYVALID;

	PHP_COUCHBASE_GET_PARAMS(couchbase_res, argflags, "l", &option);

	switch (option) {
	case COUCHBASE_OPT_SERIALIZER:
		RETURN_LONG(couchbase_res->serializer);
		break;
	case COUCHBASE_OPT_PREFIX_KEY:
		if (couchbase_res->prefix_key_len) {
			RETURN_STRINGL((char *)couchbase_res->prefix_key,
						   couchbase_res->prefix_key_len, 1);
		} else {
			ZVAL_EMPTY_STRING(return_value);
			return;
		}
		break;
	case COUCHBASE_OPT_COMPRESSION:
		RETURN_LONG(couchbase_res->compressor);
		break;
	case COUCHBASE_OPT_IGNOREFLAGS:
		RETURN_LONG(couchbase_res->ignoreflags);
		break;
	case COUCHBASE_OPT_VOPTS_PASSTHROUGH:
		RETURN_LONG(couchbase_res->viewopts_passthrough);
		break;
	default:
		if (oo) {
			char errmsg[256];
			snprintf(errmsg, sizeof(errmsg), "unknown option type: %ld",
					 option);
			zend_throw_exception(cb_illegal_option_exception,
								 errmsg, 0 TSRMLS_CC);

		} else {
			php_error_docref(NULL TSRMLS_CC, E_WARNING,
							 "unknown option type: %ld", option);
			RETURN_FALSE;
		}
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
