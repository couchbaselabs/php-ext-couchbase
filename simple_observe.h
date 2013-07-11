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
*/

#ifndef COUCHBASE_INTERNAL_H
#error "Must include internal.h"
#endif

#ifndef COUCHBASE_SIMPLE_OBSERVE_H
#define COUCHBASE_SIMPLE_OBSERVE_H

struct observe_entry {
	lcb_error_t err;
	void *key;
	int nkey;
	uint64_t cas;
	int mutated;
	int persisted;
	int replicated;
};

PHP_COUCHBASE_LOCAL
lcb_error_t simple_observe(lcb_t instance,
						   struct observe_entry *entries,
						   int nentries,
						   long persist_to,
						   long replicate_to);

PHP_COUCHBASE_LOCAL
int validate_simple_observe_clause(lcb_t instance,
								   int persist,
								   int replicas TSRMLS_DC);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
