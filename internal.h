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
#ifndef COUCHBASE_INTERNAL_H
#define COUCHBASE_INTERNAL_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#ifdef PHP_WIN32
#	 define PHP_COUCHBASE_API __declspec(dllexport)
#    define PHP_COUCHBASE_LOCAL
#	 define strtoull _strtoui64
#    define PHP_COUCHBASE_UNUSED
#    define HAVE_JSON_API 1
#    define HAVE_JSON_API_5_3 1
#elif defined(__GNUC__) && __GNUC__ >= 4
#	 define PHP_COUCHBASE_API __attribute__ ((visibility("default")))
#    define PHP_COUCHBASE_LOCAL __attribute__ ((visibility("hidden")))
#	  define PHP_COUCHBASE_UNUSED __attribute__((unused))
#else
#	 define PHP_COUCHBASE_API
#    define PHP_COUCHBASE_LOCAL
#endif

#ifdef PHP_WIN32
# include "win32/php_stdint.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/url.h"
#include "ext/standard/php_smart_str.h"
#include "ext/standard/php_var.h"
#ifdef HAVE_JSON_API
# include "ext/json/php_json.h"
#endif
#include "ext/standard/php_var.h"
#include <libcouchbase/couchbase.h>
#include "php_couchbase.h"
#include "fastlz/fastlz.h"

#ifdef HAVE_COMPRESSION_ZLIB
# include "zlib.h"
#endif

#include "Zend/zend_API.h"
#include <zend_exceptions.h>

#include "management/cluster.h"
#include "management/exceptions.h"
#include "apidecl.h"

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet expandtab sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
