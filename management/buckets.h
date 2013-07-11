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
*/
#ifndef MANAGEMENT_BUCKETS_H
#define MANAGEMENT_BUCKETS_H 1
#include "../internal.h"

PHP_COUCHBASE_LOCAL
extern void ccm_create_bucket_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern void ccm_modify_bucket_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern void ccm_delete_bucket_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern void ccm_flush_bucket_impl(INTERNAL_FUNCTION_PARAMETERS);

PHP_COUCHBASE_LOCAL
extern void ccm_get_bucket_info_impl(INTERNAL_FUNCTION_PARAMETERS);

#endif
/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
