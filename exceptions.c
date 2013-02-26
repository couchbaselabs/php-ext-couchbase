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
  | Author: Trond Norbye	<trond.norbye@couchbase.com>				 |
  +----------------------------------------------------------------------+
*/

#include "internal.h"

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_illegal_key_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_no_such_key_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_auth_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_lcb_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_server_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_key_mutated_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_timeout_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_not_enough_nodes_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_illegal_option_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_illegal_value_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_illegal_arguments_exception;

PHP_COUCHBASE_LOCAL
zend_class_entry *cb_not_supported_exception;

#define setup(var, name, parent)										\
	do {																\
		zend_class_entry cbe;											\
		INIT_CLASS_ENTRY(cbe, name, NULL);								\
		var = zend_register_internal_class_ex(&cbe, parent, NULL TSRMLS_CC); \
	} while(0)


PHP_COUCHBASE_LOCAL
void init_couchbase_exceptions(TSRMLS_D)
{
	zend_class_entry *root;

#if ZEND_MODULE_API_NO >= 20060613
	root = (zend_class_entry *)zend_exception_get_default(TSRMLS_C);
#else
	root = (zend_class_entry *)zend_exception_get_default();
#endif

	setup(cb_exception, "CouchbaseException", root);
	setup(cb_illegal_key_exception, "CouchbaseIllegalKeyException",
		  cb_exception);
	setup(cb_no_such_key_exception, "CouchbaseNoSuchKeyException",
		  cb_exception);
	setup(cb_auth_exception, "CouchbaseAuthenticationException", cb_exception);
	setup(cb_lcb_exception, "CouchbaseLibcouchbaseException", cb_exception);
	setup(cb_server_exception, "CouchbaseServerException", cb_exception);
	setup(cb_key_mutated_exception, "CouchbaseKeyMutatedException",
		  cb_exception);
	setup(cb_timeout_exception, "CouchbaseTimeoutException", cb_exception);
	setup(cb_not_enough_nodes_exception, "CouchbaseNotEnoughNodesException",
		  cb_exception);
	setup(cb_illegal_option_exception, "CouchbaseIllegalOptionException",
		  cb_exception);
	setup(cb_illegal_value_exception, "CouchbaseIllegalValueException",
		  cb_exception);
	setup(cb_illegal_arguments_exception, "CouchbaseIllegalArgumentsException",
		  cb_exception);
	setup(cb_not_supported_exception, "CouchbaseNotSupportedException",
		  cb_exception);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
