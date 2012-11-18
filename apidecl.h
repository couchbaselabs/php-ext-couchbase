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

/**
 * This file contains all the C prototypes for the *_impl functions, plus
 * some other functions called from php
 */
#ifndef COUCHBASE_INTERNAL_H
#error "Must include internal.h"
#endif

#ifndef COUCHBASE_APIDECL_H_
#define COUCHBASE_APIDECL_H_

PHP_COUCHBASE_LOCAL
void php_couchbase_res_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

PHP_COUCHBASE_LOCAL
void php_couchbase_pres_dtor(zend_rsrc_list_entry *rsrc TSRMLS_DC);

PHP_COUCHBASE_LOCAL
void php_couchbase_create_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_delayed_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_touch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_fetch_impl(INTERNAL_FUNCTION_PARAMETERS, int multi, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_store_impl(INTERNAL_FUNCTION_PARAMETERS, lcb_storage_t op, int multi, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_remove_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_flush_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_arithmetic_impl(INTERNAL_FUNCTION_PARAMETERS, char op, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_stats_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_version_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_cas_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_set_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_option_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_result_message_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_timeout_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_set_timeout_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_view_impl(INTERNAL_FUNCTION_PARAMETERS, int oo);

PHP_COUCHBASE_LOCAL
void php_couchbase_observe_impl(
		INTERNAL_FUNCTION_PARAMETERS, int multi, int oo, int poll);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_servers_impl(INTERNAL_FUNCTION_PARAMETERS, int style);

PHP_COUCHBASE_LOCAL
void php_couchbase_get_num_replicas_impl(INTERNAL_FUNCTION_PARAMETERS,
                                                int style);



#endif
