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
  | Author: Mark Nunberg       <mnunberg@haskalah.org>                   |
  +----------------------------------------------------------------------+
*/

#include "internal.h"
#include "views.h"

#define DECLARE_HANDLER(name) \
	static int name(\
			view_param *p, zval *input, pcbc_sso_buf *buf, char **error \
			TSRMLS_DC);

DECLARE_HANDLER(bool_param_handler);
DECLARE_HANDLER(num_param_handler);
DECLARE_HANDLER(string_param_handler);
DECLARE_HANDLER(jval_param_handler);
DECLARE_HANDLER(stale_param_handler);
DECLARE_HANDLER(onerror_param_handler);
DECLARE_HANDLER(jarry_param_handler);

#undef DECLARE_HANDLER

static view_param Recognized_View_Params[] = {
	{ "descending", bool_param_handler },
	{ "endkey", jval_param_handler },
	{ "endkey_docid", string_param_handler },
	{ "full_set", bool_param_handler },
	{ "group", bool_param_handler },
	{ "group_level", num_param_handler },
	{ "inclusive_end", bool_param_handler },
	{ "key", jval_param_handler },
	{ "keys", jarry_param_handler },
	{ "on_error", onerror_param_handler },
	{ "reduce", bool_param_handler },
	{ "stale", stale_param_handler },
	{ "skip", num_param_handler },
	{ "limit", num_param_handler },
	{ "startkey", jval_param_handler },
	{ "startkey_docid", string_param_handler },
	{ "debug", bool_param_handler },
	{ "connection_timeout", num_param_handler },
	{ NULL, NULL }
};


/**
 * Callbacks/Handlers for various parameters
 */
static int bool_param_handler(view_param *param,
							  zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	/**
	 * Try real hard to get a boolean value
	 */
	int bval = -1;
	*error = NULL;

	switch (Z_TYPE_P(input)) {

	case IS_BOOL:
		bval = Z_BVAL_P(input);
		break;

	case IS_LONG:
		bval = Z_LVAL_P(input);
		break;

	case IS_DOUBLE:
		bval = Z_DVAL_P(input);
		break;

	case IS_STRING:
		if (strncasecmp(
					"true", Z_STRVAL_P(input), Z_STRLEN_P(input)) == 0) {
			bval = 1;
		} else if (strncasecmp(
					   "false", Z_STRVAL_P(input), Z_STRLEN_P(input)) == 0) {
			bval = 0;
		} else {
			*error = "String must be either 'true' or 'false'";
			return -1;
		}
		break;

	case IS_NULL:
		bval = 0;
		break;

	default:
		*error = "Cannot convert parameter to boolean. "
				 "Must be int, string, bool, or NULL";
		break;
	}

	if (bval == -1) {
		return -1;
	}

	if (bval) {
		output->str = "true";
		output->len = sizeof("true") - 1;
	} else {
		output->str = "false";
		output->len = sizeof("false") - 1;
	}
	output->allocated = 0;
	return 0;
}

static int num_param_handler(view_param *param,
							 zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	/**
	 * TODO: does Zend warn about failed numeric conversion?
	 */
	zval tmp = *input;
	int is_allocated = 0;

	if (IS_STRING != Z_TYPE_P(input)) {
		if (IS_LONG != Z_TYPE_P(input)) {
			*error = "Value must be numeric";
			return -1;
		}

		zval_copy_ctor(&tmp);
		convert_to_string(&tmp);
		is_allocated = 1;
	}

	output->str = Z_STRVAL(tmp);
	output->len = Z_STRLEN(tmp);
	output->allocated = is_allocated;

	do {

		int ii;


		if (!output->len) {
			break;
		}

		for (ii = 0; ii < output->len; ii++) {
			if (!isdigit(output->str[ii])) {
				break;
			}
		}
		if (ii == output->len) {
			return 0;
		}

	} while (0);

	/**
	 * If we've reached here, we're in an error
	 */
	*error = "Value must be numeric";
	if (is_allocated) {
		zval_dtor(&tmp);
	}

	return -1;
}

static int string_param_handler(view_param *param,
								zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	zval tmp = *input;
	int encoded_len;
	int zv_is_temp = 0;

	if (Z_TYPE(tmp) == IS_STRING) {
		/* ok, nothing to do */

	} else if (Z_TYPE(tmp) == IS_LONG) {
		zval_copy_ctor(&tmp);
		convert_to_string(&tmp);
		zv_is_temp = 1;

	} else {
		*error = "Parameter must be a scalar string";
		return -1;
	}

	output->str = php_url_encode(Z_STRVAL(tmp), Z_STRLEN(tmp), &encoded_len);
	output->len = encoded_len;

	if (zv_is_temp) {
		zval_dtor(&tmp);
	}

	if (!output->str) {
		*error = "php_url_encode failed";
		return -1;
	}

	output->allocated = 1;

	return 0;
}

PHP_COUCHBASE_LOCAL
int pcbc_vopt_generic_param_handler(view_param *param,
									zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	if (Z_TYPE_P(input) == IS_STRING) {
		output->str = Z_STRVAL_P(input);
		output->len = Z_STRLEN_P(input);
		output->allocated = 0;

	} else {
		zval tmp = *input;
		zval_copy_ctor(&tmp);

		convert_to_string(&tmp);
		output->str = Z_STRVAL(tmp);
		output->len = Z_STRLEN(tmp);
		output->allocated = 1;

		ZVAL_STRINGL(&tmp, NULL, 0, 0);
		zval_dtor(&tmp);
	}
	return 0;
}

static int stale_param_handler(view_param *param,
							   zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	/**
	 * See if we can get it to be a true/false param
	 */
	if (-1 != bool_param_handler(param, input, output, error TSRMLS_CC)) {
		if (*output->str == 't') {
			output->str = "ok";
			output->len = sizeof("ok") - 1;
		}
		return 0;
	}

	if (Z_TYPE_P(input) == IS_STRING &&
			strncasecmp(
				"update_after", Z_STRVAL_P(input), Z_STRLEN_P(input)) == 0) {
		output->str = "update_after";
		output->len = sizeof("update_after") - 1;
		output->allocated = 0;
		return 0;
	}

	*error = "stale must be a boolean or the string 'update_after'";
	return -1;
}

static int onerror_param_handler(view_param *param,
								 zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	*error = "on_error must be one of 'continue' or 'stop'";
	if (IS_STRING != Z_TYPE_P(input)) {
		return -1;
	}

	if (strncasecmp(
				"stop", Z_STRVAL_P(input), Z_STRLEN_P(input)) == 0) {
		output->str = "stop";
		output->len = sizeof("stop") - 1;

	} else if (
		strncasecmp(
			"continue", Z_STRVAL_P(input), Z_STRLEN_P(input)) == -0) {
		output->str = "continue";
		output->len = sizeof("continue") - 1;

	} else {
		return -1;
	}

	output->allocated = 0;
	return 0;
}

static int jval_param_handler(view_param *param,
							  zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	smart_str buf = {0};
	zval zvtmp;
	int rv;

	pcbc_json_encode(&buf, input TSRMLS_CC);
	if (buf.c == NULL) {
		*error = "Failed to encode value as JSON";
		return -1;
	}

	ZVAL_STRINGL(&zvtmp, buf.c, buf.len, 0);
	/* We don't need to touch the zval here. It nevers owns the string */
	rv = string_param_handler(param, &zvtmp, output, error TSRMLS_CC);

	smart_str_free(&buf);
	return rv;
}

static int jarry_param_handler(view_param *param,
							   zval *input, pcbc_sso_buf *output, char **error TSRMLS_DC)
{
	if (IS_ARRAY != Z_TYPE_P(input)) {
		*error = "Parameter must be an array";
		return -1;
	}
	return jval_param_handler(param, input, output, error TSRMLS_CC);
}

PHP_COUCHBASE_LOCAL
view_param *pcbc_find_view_param(const char *vopt, size_t nvopt)
{
	view_param *ret;
	for (ret = Recognized_View_Params; ret->param; ret++) {
		if (strncmp(ret->param, vopt, nvopt) == 0) {
			return ret;
		}
	}
	return NULL;
}

PHP_COUCHBASE_LOCAL
void pcbc_sso_buf_cleanup(pcbc_sso_buf *buf)
{
	if (!buf->str) {
		return;
	}

	if (buf->allocated) {
		efree(buf->str);
	}
	buf->str = NULL;
	buf->len = 0;
	buf->allocated = 0;
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
