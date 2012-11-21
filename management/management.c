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

#include "cluster.h"
#include "instance.h"
#include "buckets.h"

PHP_METHOD(couchbaseclustermanager, __construct);
PHP_METHOD(couchbaseclustermanager, createBucket);
PHP_METHOD(couchbaseclustermanager, modifyBucket);
PHP_METHOD(couchbaseclustermanager, deleteBucket);
PHP_METHOD(couchbaseclustermanager, getBucketInfo);
PHP_METHOD(couchbaseclustermanager, getInfo);

PHP_COUCHBASE_LOCAL
int le_couchbase_cluster;
PHP_COUCHBASE_LOCAL
int le_pcouchbase_cluster;
PHP_COUCHBASE_LOCAL
zend_class_entry *couchbase_cluster_ce;

ZEND_BEGIN_ARG_INFO_EX(arginfo_construct, 0, 0, 3)
ZEND_ARG_INFO(0, host)
ZEND_ARG_INFO(0, user)
ZEND_ARG_INFO(0, password)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_create_bucket, 0, 0, 6)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, type)
ZEND_ARG_INFO(0, quota)
ZEND_ARG_INFO(0, replicas)
ZEND_ARG_INFO(0, auth)
ZEND_ARG_INFO(0, passwd)
ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_modify_bucket, 0, 0, 5)
ZEND_ARG_INFO(0, name)
ZEND_ARG_INFO(0, quota)
ZEND_ARG_INFO(0, replicas)
ZEND_ARG_INFO(0, auth)
ZEND_ARG_INFO(0, passwd)
ZEND_ARG_INFO(0, port)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_delete_bucket, 0, 0, 1)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_get_bucket_info, 0, 0, 0)
ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

static zend_function_entry methods[] = {
	PHP_ME(couchbaseclustermanager, __construct, arginfo_construct, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(couchbaseclustermanager, createBucket, arginfo_create_bucket, ZEND_ACC_PUBLIC)
	PHP_ME(couchbaseclustermanager, modifyBucket, arginfo_modify_bucket, ZEND_ACC_PUBLIC)
	PHP_ME(couchbaseclustermanager, deleteBucket, arginfo_delete_bucket, ZEND_ACC_PUBLIC)
	PHP_ME(couchbaseclustermanager, getBucketInfo, arginfo_get_bucket_info, ZEND_ACC_PUBLIC)
	PHP_ME(couchbaseclustermanager, getInfo, NULL, ZEND_ACC_PUBLIC) {
		NULL, NULL, NULL
	}
};

/* {{{ proto CouchbaseClusterManager::__construct(string $host[, string $user[, string $password]]) */
PHP_METHOD(couchbaseclustermanager, __construct)
{
	ccm_create_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto CouchbaseClusterManager::createBucket(string $name, array $meta) */
PHP_METHOD(couchbaseclustermanager, createBucket)
{
	ccm_create_bucket_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto CouchbaseClusterManager::modifyBucket(string $name, array $meta) */
PHP_METHOD(couchbaseclustermanager, modifyBucket)
{
	ccm_modify_bucket_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto Couchbaseclustermanager::deleteBucket(string $name) */
PHP_METHOD(couchbaseclustermanager, deleteBucket)
{
	ccm_delete_bucket_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

/* {{{ proto Couchbaseclustermanager::getBucketInfo([string $name]) */
PHP_METHOD(couchbaseclustermanager, getBucketInfo)
{
	ccm_get_bucket_info_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */


/* {{{ proto Couchbaseclustermanager::getInfo(void) */
PHP_METHOD(couchbaseclustermanager, getInfo)
{
	ccm_get_info_impl(INTERNAL_FUNCTION_PARAM_PASSTHRU);
}
/* }}} */

static void resource_destructor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
	lcb_t instance = (lcb_t)rsrc->ptr;
	if (instance) {
		lcb_destroy(instance);
	}
}

static void persistent_resource_destructor(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
}

PHP_COUCHBASE_LOCAL
void init_couchbase_cluster(int module_number TSRMLS_DC)
{
	le_couchbase_cluster = zend_register_list_destructors_ex(resource_destructor, NULL,
															 PHP_COUCHBASE_CLUSTER_RESOURCE,
															 module_number);
	le_pcouchbase_cluster = zend_register_list_destructors_ex(NULL,
															  persistent_resource_destructor,
															  PHP_COUCHBASE_CLUSTER_PERSISTENT_RESOURCE,
															  module_number);

	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "CouchbaseClusterManager", methods);
	couchbase_cluster_ce = zend_register_internal_class_ex(&ce, NULL,
														   NULL TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
