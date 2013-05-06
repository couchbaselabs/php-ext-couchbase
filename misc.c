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

void php_couchbase_get_servers_impl(INTERNAL_FUNCTION_PARAMETERS,
									int style)
{
	php_couchbase_res *res;
	const char *const *server_list = NULL;
	const char *const *cur_item;
	int wix = 0;
	PHP_COUCHBASE_GET_PARAMS(res, style, "");

	server_list = lcb_get_server_list(res->handle);

	array_init(return_value);

	for (wix = 0, cur_item = server_list;
			*cur_item;
			cur_item++, wix++) {

		add_index_string(return_value, wix, *cur_item, 1);
	}
}


void php_couchbase_get_num_replicas_impl(INTERNAL_FUNCTION_PARAMETERS,
										 int style)
{
	php_couchbase_res *res;
	PHP_COUCHBASE_GET_PARAMS(res, style, "");
	ZVAL_LONG(return_value, lcb_get_num_replicas(res->handle));
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
