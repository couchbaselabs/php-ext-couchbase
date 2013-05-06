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
  | Author: Trond Norbye       <trond.norbye@gmail.org>                  |
  +----------------------------------------------------------------------+
*/
#include "internal.h"

static const char errmsg[] = "Failed to allocate buffer for error message";

PHP_COUCHBASE_LOCAL
void couchbase_report_error(INTERNAL_FUNCTION_PARAMETERS, int oo,
							zend_class_entry *exception, const char *fmt, ...)
{
	/* No error messages should be longer than this! */
#define MAX_ERROR_MSG_LENGTH 4096
	char *msg = malloc(MAX_ERROR_MSG_LENGTH);
	va_list ap;

	if (msg == NULL) {
		msg = (char *)errmsg;
	} else {
		/* format the error message */
		va_start(ap, fmt);
		vsnprintf(msg, MAX_ERROR_MSG_LENGTH, fmt, ap);
		va_end(ap);
	}

	if (oo) {
		zend_throw_exception(exception, msg, 0 TSRMLS_CC);
	} else {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "%s", msg);
		RETVAL_FALSE;
	}

	if (msg != (char *)errmsg) {
		free(msg);
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
