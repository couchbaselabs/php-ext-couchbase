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
  | Author: Trond Norbye   <trond.norbye@couchbase.com>					 |
  +----------------------------------------------------------------------+
*/
#ifndef MANAGEMENT_EXCEPTIONS_H
#define MANAGEMENT_EXCEPTIONS_H
#include "../internal.h"

PHP_COUCHBASE_LOCAL
void init_couchbase_exceptions(TSRMLS_D);

PHP_COUCHBASE_LOCAL
extern zend_class_entry *cb_exception;

PHP_COUCHBASE_LOCAL
extern zend_class_entry *cb_illegal_key_exception;

PHP_COUCHBASE_LOCAL
extern zend_class_entry *cb_auth_exception;

PHP_COUCHBASE_LOCAL
extern zend_class_entry *cb_lcb_exception;

PHP_COUCHBASE_LOCAL
extern zend_class_entry *cb_server_exception;

#endif	  /* MANAGEMENT_EXCEPTION_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
