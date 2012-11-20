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
#ifndef MANAGEMENT_CLUSTER_H
#define MANAGEMENT_CLUSTER_H

#define PHP_COUCHBASE_CLUSTER_RESOURCE	  "CouchbaseCluster"
#define PHP_COUCHBASE_CLUSTER_PERSISTENT_RESOURCE "Persistent Couchbase Cluster"
#define COUCHBASE_PROPERTY_HANDLE "_handle"

#include "../internal.h"


PHP_COUCHBASE_LOCAL
void init_couchbase_cluster(int module_number TSRMLS_DC);

#endif	  /* MANAGEMENT_CLUSTER_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
