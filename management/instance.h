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
#ifndef MANAGEMENT_INSTANCE_H
#define MANAGEMENT_INSTANCE_H 1
#include "../internal.h"

PHP_COUCHBASE_LOCAL
extern void ccm_create_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern void ccm_get_info_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern int le_couchbase_cluster;

PHP_COUCHBASE_LOCAL
extern int le_pcouchbase_cluster;

PHP_COUCHBASE_LOCAL
extern zend_class_entry *couchbase_cluster_ce;

struct lcb_http_ctx {
	lcb_error_t error;
	lcb_http_status_t status;
	char *payload;
    int use_emalloc;
};

#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
